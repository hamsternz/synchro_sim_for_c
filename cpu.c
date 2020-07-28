#include <stdint.h>
#include <stdio.h>
#include <memory.h>

#include "config.h"
#include "bus.h"
#include "io.h"

enum Phase {fetch_0 = 0, fetch_1, jump, load, load_1, store, alu_immed, halt};

// Values that need to persist from clock cycle to clock cycle.
struct State {
  uint8_t  regs[4]; // Unused reg, reg_a, reg_b and reg_c
  enum     Phase phase;
  uint32_t program_counter;
  uint8_t  loadstore_reg;
  uint8_t  immed_tgt_reg;
  uint8_t  immed_operation;
  uint8_t  load_target_reg;
  uint8_t  low_nibble;
};

static struct State state;
static struct State next_state;
static int debug;

void cpu_debug(int i) {
   debug = i;
}

void cpu_calc_next_state(void) {
  char    op = '?';
  uint8_t arg1 = 0, arg2 = 0, res = 0;
  uint8_t alu_op_tgt_reg, alu_op_src_reg, alu_op_operation;

  ///////////////////////////////////////////////////////////////////////
  // Force the bus to safe values - overwrite later for bus operation
  ///////////////////////////////////////////////////////////////////////
  bus_io_select     = 0; 
  bus_mem_select    = 0; 
  bus_write_enable  = 0;

  ////////////////////////////////////////////////
  // Decoding for the first byte of instuctions
  ////////////////////////////////////////////////
  enum Inst_Type {inst_unknown, inst_halt, inst_load, inst_store, inst_jump, inst_immed, inst_reg_reg} inst_type = inst_unknown;

  switch(bus_data_in & 0xC0) {
     case 0xC0: inst_type = inst_store; break;
     case 0x80: inst_type = inst_load;  break;
     case 0x40: inst_type = inst_jump;  break;
     case 0x00: if(bus_data_in == 0)
		   inst_type = inst_halt;
		else if(bus_data_in & 0x30)
		   inst_type = inst_reg_reg;
		else
	           inst_type = inst_immed;
		break;
  }
  // Bits we need for single cycle instructions
  alu_op_tgt_reg   = (bus_data_in>>2) & 3;
  alu_op_src_reg   = (bus_data_in>>0) & 3;
  alu_op_operation = (bus_data_in>>4) & 3;
  // Bits we need to remember for second cycle of instruction
  next_state.loadstore_reg    = (bus_data_in>>4) & 3;
  next_state.immed_operation  = (bus_data_in>>4) & 3;
  next_state.immed_tgt_reg    = (bus_data_in>>2) & 3;
  next_state.low_nibble       = bus_data_in & 0xF;
  // Bits we need to remember for the third cycle of instruction
  next_state.load_target_reg  = state.loadstore_reg;

  /////////////////////////////////////////////////////////////////
  // Doing different things for different phases of the instruction
  /////////////////////////////////////////////////////////////////
  switch(state.phase) {
     case fetch_0: // fetch first byte of instruction from memory
        bus_mem_select             = 1;
        bus_addr                   = state.program_counter+0;
        next_state.phase           = fetch_1;
	if(debug) printf("PC = %06x\n",state.program_counter);
        break;

     case fetch_1: // The main part of the instruction
	// We have first byte of instuction, so decide what to do
	switch(inst_type) {
	   case inst_halt: // Halt the CPU
              if(debug) printf("Halt\n");
              next_state.phase           = halt;
	      break;
  	   case inst_load: // Load/stores are two bytes
              bus_mem_select             = 1;
              bus_addr                   = state.program_counter+1;
              next_state.phase           = load;
	      break;
  	   case inst_store: // Load/stores are two bytes
              bus_mem_select             = 1;
              bus_addr                   = state.program_counter+1;
              next_state.phase           = store;
	      break;
	   case inst_jump: // Jumps are two bytes
              bus_mem_select             = 1;
              bus_addr                   = state.program_counter+1;
              next_state.phase           = jump;
	      break;
	   case inst_immed: // Immediates are two bytes
              bus_mem_select             = 1;
              bus_addr                   = state.program_counter+1;
              next_state.phase           = alu_immed;
	      break;
	   case inst_reg_reg: // Reg to Reg ALU operation
              next_state.program_counter  = state.program_counter+1;
              next_state.phase            = fetch_0;

	      arg1 = state.regs[alu_op_tgt_reg];
	      arg2 = state.regs[alu_op_src_reg];
	      switch(alu_op_operation) {
	          case 0: res = arg1 + arg2; op = '+'; break;
	          case 1: res = arg1 - arg2; op = '-'; break;
	          case 2: res = arg1 & arg2; op = '$'; break;
	          case 3: res = arg1 | arg2; op = '|'; break;
	      }
	      next_state.regs[alu_op_tgt_reg] = res;
              if(debug) printf("ALU reg_%c = reg_%c %c reg_%c   =  %02x\n", alu_op_tgt_reg+'a'-1, alu_op_tgt_reg+'a'-1, op, alu_op_src_reg+'a', res);
	      break;
	   default:
	      printf("Unknown instruction\n");
              next_state.phase            = halt;
	      break;
	}
        break;

     case jump: // This is a two-byte JUMP instruction
	// Should be using bits 5:$ of the instuction to decide when to do jump
        if(debug) printf("Jump %i\n", (state.low_nibble<<8) | bus_data_in);
	next_state.program_counter  = state.program_counter+2;
        next_state.phase            = fetch_0;
	break;

     case load: // This is a two-byte load instruction - put the address to read on the bus
	next_state.program_counter  = state.program_counter+2;
	if(state.low_nibble == 0 && bus_data_in < 16)
          bus_io_select               = 1;
	else
          bus_mem_select              = 1;
        bus_addr                    = (state.low_nibble <<8) | bus_data_in;
	if(debug) printf("Read at %03x\n", bus_addr);
        next_state.phase        = load_1;
        break;

     case load_1: // The value to be loaded is on the bus
	next_state.regs[state.load_target_reg] = bus_data_in;
        next_state.phase        = fetch_0;
        break;

     case store: // Place the address and data to be written on the bus 
	next_state.program_counter  = state.program_counter+2;
	if(state.low_nibble == 0 && bus_data_in < 16)
          bus_io_select   = 1;
	else
          bus_mem_select  = 1;
        bus_addr          = (state.low_nibble <<8) | bus_data_in;
        bus_write_enable  = 1;
	bus_data_out      = state.regs[state.loadstore_reg];
        if(debug) printf("Write %02x at %03x\n", bus_data_out, bus_addr);
        next_state.phase        = fetch_0;
        break;

     case alu_immed: // An ALU immediate operation
	next_state.program_counter  = state.program_counter+2;
        next_state.phase            = fetch_0;
        arg1 = state.regs[state.immed_tgt_reg];
        arg2 = bus_data_in;
        switch(state.immed_operation) {
           case 0: res = arg1 + arg2; op = '+'; break;
           case 1: res = arg1 - arg2; op = '-'; break;
           case 2: res = arg1 & arg2; op = '&'; break;
           case 3: res = arg1 | arg2; op = '|'; break;
        }
        next_state.regs[state.immed_tgt_reg] = res;
        if(debug) printf("ALU reg_%c = reg_%c %c x%02x\n", state.immed_tgt_reg+'a'-1, state.immed_tgt_reg+'a'-1, op, bus_data_in);
        break;

     case halt:
	break;

     default:
	if(debug) printf("unknown state\n");
        next_state.phase           = fetch_0;
        break;
  }
}

void cpu_clock(void) {
   /* Make the next_state the current state, equivilent to a clock tick */
   memcpy(&state, &next_state, sizeof(state));
}

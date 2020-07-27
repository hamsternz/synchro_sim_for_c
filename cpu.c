#include <stdint.h>
#include <stdio.h>
#include <memory.h>

#include "config.h"
#include "bus.h"
#include "io.h"

enum Phase {fetch_0 = 0, fetch_1,
	    jump,       
	    loadstore, load_1, 
	    alu_immed,
            halt};

struct State {
  uint8_t  reg_a;
  uint8_t  reg_b;
  uint8_t  reg_c;
  uint32_t instruction;
  enum     Phase phase;
  uint32_t program_counter;
};
static struct State state;
static struct State next_state;
static int debug;

void cpu_debug(int i) {
   debug = i;
}

void cpu_calc_next_state(void) {
  uint32_t a;
  /*********************************************************
  * In this you can only set the state of the bus signals:
  *
  * - bus_io_select 
  * - bus_mem_select 
  * - bus_write_enable
  * - bus_addr
  * - bus_data_out
  * - Or elements in next_state 
  * 
  * This is equivilent to combinatorial logic in the CPU
  *********************************************************/

  /* Force the bus to safe valuess */
  bus_io_select     = 0; 
  bus_mem_select    = 0; 
  bus_write_enable  = 0;

  //if(debug) printf("Phase %i\n",state.phase);

  // Continually capture the data bus to buld up mulri-word instructions
  next_state.instruction  = (state.instruction << 8) | bus_data_in;
  a                       = ((state.instruction << 8) | bus_data_in) & 0xFFF;

  switch(state.phase) {
     case fetch_0:
        /* fetch from memory */
        bus_mem_select             = 1;
        bus_addr                   = state.program_counter+0;
        next_state.phase           = fetch_1;
	if(debug) printf("PC = %06x\n",state.program_counter);
        break;

     case fetch_1:
	if(bus_data_in == 0) {
           if(debug) printf("Halt\n");
           next_state.phase           = halt;
	} else if(bus_data_in & 0x80) {
	   // Load/stores are two bytes
           bus_mem_select             = 1;
           bus_addr                   = state.program_counter+1;
           next_state.phase           = loadstore;
        } else if(bus_data_in & 0x40) {
	   // Jumps are two bytes
           bus_mem_select             = 1;
           bus_addr                   = state.program_counter+1;
           next_state.phase           = jump;
        } else if((bus_data_in & 3) == 0) {
	   // Imediates are two bytes
           bus_mem_select             = 1;
           bus_addr                   = state.program_counter+1;
           next_state.phase           = alu_immed;
	} else {
           uint8_t arg1 = 0, arg2 = 0, res = 0;
           char reg1 = '?', reg2 = '?', op = '?';
   	   next_state.program_counter  = state.program_counter+1;
           next_state.phase            = fetch_0;
	   switch(bus_data_in & 0xC) {
	       case 0x4: arg1 = state.reg_a; reg1 = 'a'; break;
	       case 0x8: arg1 = state.reg_b; reg1 = 'b'; break;
	       case 0xC: arg1 = state.reg_c; reg1 = 'c'; break;
	   }
	   switch(bus_data_in & 0x3) {
	       case 0x0: printf("Unknown opcode\n");     break;
	       case 0x1: arg2 = state.reg_a; reg2 = 'a'; break;
	       case 0x2: arg2 = state.reg_b; reg2 = 'b'; break;
	       case 0x3: arg2 = state.reg_c; reg2 = 'c'; break;
	   }
	   switch(bus_data_in & 0x30) {
	       case 0x00: res = arg1 + arg2; op = '+'; break;
	       case 0x10: res = arg1 - arg2; op = '-'; break;
	       case 0x20: res = arg1 & arg2; op = '$'; break;
	       case 0x30: res = arg1 | arg2; op = '|'; break;
	   }
	   switch(bus_data_in & 0xC) {
	       case 0x4: next_state.reg_a = res; break;
	       case 0x8: next_state.reg_b = res; break;
	       case 0xC: next_state.reg_c = res; break;
	   }
           if(debug) printf("ALU reg_%c = reg_%c %c reg_%c   =  %02x\n", reg1, reg1, op, reg2, res);
        }
        break;

     case jump:
        if(debug) printf("Jump %i\n",a);
	next_state.program_counter  = state.program_counter+2;
        next_state.phase            = fetch_0;
	break;

     case loadstore:
	next_state.program_counter  = state.program_counter+2;
	if(a < 16)
          bus_io_select               = 1;
	else
          bus_mem_select              = 1;
        bus_addr                    = a;
	if(state.instruction & 0x40) {
           bus_write_enable        = 1;
	   switch(state.instruction & 0x30) {
	      case 0x10: bus_data_out = state.reg_a; break;
	      case 0x20: bus_data_out = state.reg_b; break;
	      case 0x30: bus_data_out = state.reg_c; break;
	      default:   bus_data_out = 0;           break;
	   }
	   if(debug) printf("Write %02x at %03x\n", bus_data_out, a);
           next_state.phase        = fetch_0;
	}
	else {
	   if(debug) printf("Read at %03x\n",a);
           next_state.phase        = load_1;
	}
        break;

     case load_1:
	switch((state.instruction>>8) & 0x4) {
	   case 1:  state.reg_a = bus_data_in; break;
	   case 2:  state.reg_b = bus_data_in; break;
	   case 3:  state.reg_c = bus_data_in; break;
	   default: break;
        }
        next_state.phase        = fetch_0;
        break;

     case alu_immed:
	next_state.program_counter  = state.program_counter+2;
        next_state.phase            = fetch_0;
	{
           uint8_t arg1 = 0, arg2 = 0, res = 0;
	   char rdest = '?', op = '?';
	   switch(state.instruction & 0xC) {
	       case 0x4: arg1 = state.reg_a; rdest = 'a'; break;
	       case 0x8: arg1 = state.reg_b; rdest = 'b'; break;
	       case 0xC: arg1 = state.reg_c; rdest = 'c'; break;
	   }
	   arg2 = bus_data_in;
	   switch(state.instruction & 0x30) {
	       case 0x00: res = arg1 + arg2; op = '+'; break;
	       case 0x10: res = arg1 - arg2; op = '-'; break;
	       case 0x20: res = arg1 & arg2; op = '&'; break;
	       case 0x30: res = arg1 | arg2; op = '|'; break;
	   }
	   switch(state.instruction & 0xC) {
	       case 0x4: next_state.reg_a = res; break;
	       case 0x8: next_state.reg_b = res; break;
	       case 0xC: next_state.reg_c = res; break;
	   }
           if(debug) printf("ALU reg_%c = reg_%c %c x%02x\n", rdest, rdest, op, bus_data_in);
        }
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


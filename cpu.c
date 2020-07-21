#include <stdint.h>
#include <stdio.h>
#include <memory.h>

#include "config.h"
#include "bus.h"
#include "io.h"

struct State {
  uint32_t phase;
  uint32_t reg;
  uint32_t program_counter;
};
static struct State state;
static struct State next_state;
static int debug;

void cpu_debug(int i) {
   debug = i;
}

void cpu_calc_next_state(void) {
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

  if(debug) printf("Phase %i\n",state.phase);
  switch(state.phase) {
     case 0:
        /* fetch from memory */
        bus_mem_select   = 1;
        bus_addr         = state.program_counter;

        next_state.phase = 1;
        next_state.program_counter = state.program_counter + 1;
        break;

     case 1:
        /* Save it into a register */
        next_state.reg   = bus_data_in;
        next_state.phase = 2;
        break;
      
     case 2:
        /* Write it to address zero of the IO space */
        bus_io_select    = 1;
        bus_write_enable = 1;
        bus_addr         = 0;
        bus_data_out     = state.reg;

	next_state.reg   = state.reg >>8;
        next_state.phase = 3;
        break;

     case 3:
        /* Write it to address zero of the IO space */
        bus_io_select    = 1;
        bus_write_enable = 1;
        bus_addr         = 0;
        bus_data_out     = state.reg;

	next_state.reg   = state.reg >>8;
        next_state.phase = 4;
        break;

     case 4:
        /* Write it to address zero of the IO space */
        bus_io_select    = 1;
        bus_write_enable = 1;
        bus_addr         = 0;
        bus_data_out     = state.reg;

	next_state.reg   = state.reg >>8;
        next_state.phase = 5;
        break;

     case 5:
        /* Write it to address zero of the IO space */
        bus_io_select    = 1;
        bus_write_enable = 1;
        bus_addr         = 0;
        bus_data_out     = state.reg;

        next_state.phase = 0;
        break;

     default:
        next_state.phase = 0;
        break;
  }
}

void cpu_clock(void) {
   /* Make the next_state the current state, equivilent to a clock tick */
   memcpy(&state, &next_state, sizeof(state));
}


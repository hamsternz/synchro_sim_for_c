#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "bus.h"
#include "io.h"
#include "term.h"

#define IO_SIZE 4096

#define IO_SERIAL_DATA  0

static int debug = 0;

void io_debug(int i) {
   debug = i;
}

void io_calc_next_state(void) {
}

void io_clock(void) {
   if(bus_io_select) {
      if(bus_write_enable) {
         switch(bus_addr&IO_SIZE) {
             case IO_SERIAL_DATA:
                 if(debug) {
		    printf("Write to Serial %08x  %c\n",bus_data_out, bus_data_out > 27 && bus_data_out < 127 ? bus_data_out : '?');
		 } else {
                    term_char_write(bus_data_out);
                 }
		 break;
         }
      } else {
         switch(bus_addr&IO_SIZE) {
             case IO_SERIAL_DATA:
                 if(debug) {
		    printf("Read to Serial\n");
		 }
		 bus_data_in = term_char_read();
		 break;
         }
      }
   }	
}


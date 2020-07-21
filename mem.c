#include <stdio.h>
#include <stdint.h>

#include "config.h"
#include "bus.h"
#include "mem.h"

// Size in 32-bit words
#define MEM_SIZE 1024  

static uint32_t data[MEM_SIZE];
static int debug;

int mem_load(char *filename) {
   FILE *f;
   int n_read;

   printf("Loading memory from '%s'\n", filename);

   f = fopen(filename, "r");
   if(f == NULL) {
      return 0;
   }

// n_read = fread(data, sizeof(uint32_t), MEM_SIZE, f);
   n_read = fread(data, 1, sizeof(data), f);
   fclose(f);

   if(n_read == 0) {
      printf("Unable to read any memory\n");
      return 0;
   }
   printf("Read %i words into memory\n", n_read);
   fclose(f);
   return 1;
}

void mem_debug(int i) {
   debug = i;
}

void mem_calc_next_state(void) {
   // Not used for memory
}

void mem_clock(void) {
   if(bus_mem_select) {
   if(bus_write_enable) {
	 if(debug) printf("Mem write %08x %08x\n", bus_addr, bus_data_out);
         data[bus_addr&(MEM_SIZE-1)] = bus_data_out;
      } else {
         bus_data_in  = data[bus_addr&(MEM_SIZE-1)];
	 if(debug) printf("Mem read %08x %08x\n", bus_addr, bus_data_in);
      }
   }	
}


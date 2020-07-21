#include <stdio.h>
#include <stdint.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>


#include "config.h"
#include "bus.h"
#include "mem.h"
#include "cpu.h"
#include "io.h"
#include "term.h"

int main(int argc, char *argv[]) {
   int i;

   /* Check tha the memory image has been supplied and load it */
   if(argc != 2) {
     fprintf(stderr, "Must supply memory image name\n");
     return 0;
   }
   if(!mem_load(argv[1])) {
     fprintf(stderr, "Error loading memory image\n");
     return 0;
   }

   term_setup();

   /* Set the debug options */
   mem_debug(0);
   io_debug(0);
   cpu_debug(0);

   for(i = 0; i < 5000; i++) {
      /* Calcuate all the next state */
      cpu_calc_next_state();
      io_calc_next_state();
      mem_calc_next_state();

      /* Make the clock tick */
      cpu_clock();
      io_clock();
      mem_clock();
   }
   printf("%i cycles done\n",i);
   term_cleanup();
}

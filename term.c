#include <stdio.h>
#include <stdint.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "config.h"
#include "term.h"

struct termios ttystate, ttysave;

int term_char_read(void) {
   struct timeval tv;
   fd_set readset;

   tv.tv_sec = 0;
   tv.tv_usec = 0;

   FD_ZERO(&readset);
   FD_SET(fileno(stdin), &readset);
   select(fileno(stdin)+1, &readset, NULL, NULL, &tv);

   if(!FD_ISSET(fileno(stdin), &readset))
     return -1;

   return getc(stdin);
}

void term_char_write(int i) {
   putchar(i);
}

int term_setup(void) {
   tcgetattr(STDIN_FILENO, &ttystate);
   ttysave = ttystate;
   //turn off canonical mode and echo
   ttystate.c_lflag &= ~(ICANON | ECHO);

   //minimum of number input read.
   ttystate.c_cc[VMIN] = 1;
   //set the terminal attributes.
   tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
   return 1;
}

void term_cleanup(void) {
   ttystate.c_lflag |= ICANON | ECHO;
   //set the terminal attributes.
   tcsetattr(STDIN_FILENO, TCSANOW, &ttysave);
}

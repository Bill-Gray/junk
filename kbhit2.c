/* Based on the "general" version of kbhit() at

http://flipcode.com/archives/_kbhit_for_Linux.shtml

   Note that 'echo' is turned off,  and the terminal is reset to
its original state when we exit.  Unless your program aborts.
Some use of atexit( ) may be appropriate.       */

#include <sys/select.h>
#include <stdio.h>
#include <termios.h>

#define STDIN  0

int kbhit( void)
{
   struct timeval timeout;
   fd_set rdset;

   FD_ZERO( &rdset);
   FD_SET( STDIN, &rdset);
   timeout.tv_sec = 0;
   timeout.tv_usec = 50000;
   return( select( STDIN + 1, &rdset, NULL, NULL, &timeout));
}

/* The following reassures the compiler that it need not warn us
that some parameters (here,  argc and argv) are not used. */

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

int main( const int argc, const char **argv)
{
   int c = 0;
   // Use termios to turn off line buffering
   struct termios term, old_term;

   INTENTIONALLY_UNUSED_PARAMETER( argv);
   INTENTIONALLY_UNUSED_PARAMETER( argc);
   tcgetattr( STDIN, &term);
   old_term = term;
   term.c_lflag &= ~(ICANON | ECHO);
   tcsetattr( STDIN, TCSANOW, &term);

   setbuf( stdin, NULL);
   setbuf( stdout, NULL);
   while( c != 'q')
      {
      if( !kbhit( ))
         printf( ".");
      else
         {
         c = getchar( );
         printf( " %c (%d)\n", c, c);
         }
      }
   tcsetattr(STDIN, TCSANOW, &old_term);
   return( 0);
}

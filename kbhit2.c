/* Based in part on... */
/* http://flipcode.com/archives/_kbhit_for_Linux.shtml */

#include <sys/select.h>
#include <stdio.h>

int kbhit( void)
{
   struct timeval timeout;
   fd_set rdset;
   const int STDIN = 0;

   FD_ZERO( &rdset);
   FD_SET( STDIN, &rdset);
   timeout.tv_sec = 0;
   timeout.tv_usec = 50000;
   return( select( STDIN + 1, &rdset, NULL, NULL, &timeout));
}

int main( const int argc, const char **argv)
{
   int c = 0;

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
   return( 0);
}

#include <stdio.h>

/* Code to generate the ASCII table at

https://www.projectpluto.com/temp/ascii.htm

Goes a bit beyond ASCII to show some low-end Unicode.  */

int main( const int argc, const char **argv)
{
   int i;

   for( i = 32; i < 512; i++)
      if( i < 128 || i >= 160)
         {
         printf( "%3d/%3x: &#x%x;%s", i, (unsigned)i, (unsigned)i,
                        (i % 8 == 7 ? "\n" : "    "));
         }
   return( 0);
}

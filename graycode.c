#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Modified from _Numeric Recipes_,  20.2.  Converts to/from the
Gray (no relation) code system in which one bit flips on consecutive
values.  The sequence starts with...

0:  0 00000     8:  c 01100    16: 18 11000     24: 14 10100    32: 10 10000
1:  1 00001     9:  d 01101    17: 19 11001     25: 15 10101    33: 11 10001
2:  3 00011    10:  f 01111    18: 1b 11011     26: 17 10111    34: 13 10011
3:  2 00010    11:  e 01110    19: 1a 11010     27: 16 10110    35: 12 10010
4:  6 00110    12:  a 01010    20: 1e 11110     28: 12 10010    36: 16 10110
5:  7 00111    13:  b 01011    21: 1f 11111     29: 13 10011    37: 17 10111
6:  5 00101    14:  9 01001    22: 1d 11101     30: 11 10001    38: 15 10101
7:  4 00100    15:  8 01000    23: 1c 11100     31: 10 10000    39: 14 10100
*/

int gray_code( const int ival)
{
   return( ival ^ (ival >> 1));
}

int rev_gray_code( int ival)
{
   int rval;

   for( rval = 0; ival; ival >>= 1)
      rval ^= ival;
   return( rval);
}

int main( const int argc, const char **argv)
{
   int i, end_i = atoi( argv[2]);

   for( i = atoi( argv[1]); i <= end_i; i++)
      {
      const int gc = gray_code( i);
      int j;

      printf( "%4d: %4x ", i, gc);
      for( j = 256; j; j >>= 1)              /* show binary form */
         printf( "%d", (gc & j) ? 1 : 0);    /* why is there no %b format flag? */
      printf( "\n");
      assert( i == rev_gray_code( gc));
      }
   return( 0);
}

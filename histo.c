#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Code to take a list of numbers from a file and make a text-based
histogram of them.  I wrote this when I had a slew of errors running
from about -.2 to +.2,  and wanted a way to see them as a histogram
with 0.01 bins.  Run as

./histo input_file.txt 0.0025

   this spat out a set of lines such as

-0.170 ****
-0.160
-0.150
-0.140 *
-0.130 **
-0.120 *
-0.110 **
-0.100 **
-0.090 ***
-0.080 ****
-0.070 *****
-0.060 ****
-0.050 *******
-0.040 **********

   ...and so on,  up to 0.160.  It's assumed that each line of the input
starts with a number;  you can use '-c' to tell the program to look in a
certain number of characters on each line.
*/

#define IS_POWER_OF_TWO( n)    (((n) & ((n)-1)) == 0)

int main( const int argc, const char **argv)
{
   int i, j, n_found = 0, column = 0;
   int low_bin, high_bin, max_count;
   FILE *ifile = fopen( argv[1], "rb");
   char buff[1000];
   double *ivals = NULL, ival, spacing = atof( argv[2]);
   double min_val = 0., max_val = 0.;
   int *counts;

   assert( ifile);
   for( i = 3; i < argc; i++)
      if( argv[i][0] == '-')
         switch( argv[i][1])
            {
            case 'c':
               column = atoi( argv[i] + 2);
               break;
            }
   while( fgets( buff, sizeof( buff), ifile))
      if( strlen( buff) > (size_t)column && sscanf( buff + column, "%lf", &ival) == 1)
         {
         if( !n_found || max_val < ival)
            max_val = ival;
         if( !n_found || min_val > ival)
            min_val = ival;
         n_found++;
         if( IS_POWER_OF_TWO( n_found))
            ivals = (double *)realloc( ivals, 2 * n_found * sizeof( double));
         ivals[n_found - 1] = ival;
         }
   fclose( ifile);
   assert( n_found);
   low_bin = (int)floor( min_val / spacing);
   high_bin = (int)ceil( max_val / spacing);
   counts = (int *)calloc( high_bin - low_bin, sizeof( int));
   assert( counts);
   for( i = 0; i < n_found; i++)
      counts[(int)floor( ivals[i] / spacing) - low_bin]++;
   free( ivals);
   for( i = max_count = 0; i < high_bin - low_bin; i++)
      if( max_count < counts[i])
         max_count = counts[i];
   for( i = max_count = 0; i < high_bin - low_bin; i++)
      {
      printf( "%7.4f ", (double)(low_bin + i) * spacing);
      j = counts[i];
      if( max_count > 40)
         j = j * 40 / max_count;
      while( j--)
         printf( "*");
      printf( "\n");
      }
   free( counts);
   return( 0);
}

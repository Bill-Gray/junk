#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

/* Code to read 'before.txt' and 'after.txt' vector ephems from
Find_Orb,  and print out for each ephemeris step the difference in
position (total distance) and the difference in velocity.  The minimum
point ought to correspond to where a maneuver took place. Compile with

gcc -Wall -Wextra -pedantic -o delta_v delta_v.c -lm     */


int main( const int argc, const char **argv)
{
   FILE *fp1 = fopen( (argc < 2 ? "before.txt" : argv[1]), "rb");
   FILE *fp2 = fopen( (argc < 3 ? "after.txt" : argv[2]), "rb");
   char buff1[200], buff2[200];
   const char *header_trailer =
"  JDE             delta(km)    deltavx  deltavy  deltavz  delta_v (m/s)\n";

   assert( fp1);
   assert( fp2);
   if( !fgets( buff1, sizeof( buff1), fp1) || !fgets( buff2, sizeof( buff2), fp2))
      exit( -1);
   printf( "%s", header_trailer);
   while( fgets( buff1, sizeof( buff1), fp1) && fgets( buff2, sizeof( buff2), fp2)
                  && strlen( buff1) > 3)
      {
      double delta, diff = 0., delta_v_squared = 0.;
      size_t i;

      for( i = 14; i < 57; i += 21)
         {
         delta = atof( buff1 + i) - atof( buff2 + i);
         diff += delta * delta;
         }
      printf( "%.14s %13.5f ", buff1, sqrt( diff));
      for( i = 78; i < 139; i += 21)
         {
         const double delta_in_km_sec = atof( buff2 + i) - atof( buff1 + i);

         printf( "%+9.3f", 1000. * delta_in_km_sec);
         delta_v_squared += delta_in_km_sec * delta_in_km_sec;
         }
      printf( "%10.3f\n", 1000. * sqrt( delta_v_squared));
      }
   printf( "%s", header_trailer);
   fclose( fp1);
   fclose( fp2);
}

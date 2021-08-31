#include <stdint.h>
#include <stddef.h>

/* Code to compute sines and cosines (simultaneously) in fixed-point
math,  using only addition,  subtraction,  and bit-shifting,  using
the CORDIC algorithm :

https://en.wikipedia.org/wiki/CORDIC         */

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923

static int16_t tbl[14] = { 8192, 4836, 2555, 1297, 651, 326,
                           163, 81, 41, 20, 10, 5, 3, 1 };
static int16_t starting_value = 9949;

#ifdef CORDIC_TABLE_GENERATOR

/* The above values for the CORDIC sine/cosine function were
computed using the following snippet.  */

#include <math.h>
#include <stdio.h>

static void compute_cordic_table( void)
{
   size_t i;
   double tangent = 1.0, rescale = 1.;

   for( i = 0; i < 14; i++, tangent /= 2.)
      {
      tbl[i] = (int16_t)( atan( tangent) * 32768. / PI + 0.5);
      rescale *= sqrt( 1. + tangent * tangent);
      printf( "%d: %u\n", i, tbl[i]);
      }
   starting_value = (int16_t)( 16384. / rescale);
   printf( "Start: %d\n", starting_value);
}
#endif         /* #ifdef CORDIC_TABLE_GENERATOR */

static void cordic_sincos( int16_t angle, int16_t *sine, int16_t *cosine)
{
   size_t i;

   if( angle > 16384 || angle < -16384)
      {       /* beyond +/- 90 degrees */
      *cosine = -starting_value;
      *sine = 0;
      angle += (int16_t)-32768;
      }
   else
      {
      *cosine = starting_value;
      *sine = 0;
      }
   for( i = 0; i < 14; i++)
      {
      const int16_t tval = *cosine;

      if( angle > 0)
         {
         *cosine -= (*sine) >> i;
         *sine += tval >> i;
         angle -= tbl[i];
         }
      else
         {
         *cosine += (*sine) >> i;
         *sine -= tval >> i;
         angle += tbl[i];
         }
      }
}

#define TWO_PI (PI + PI)

static void cosine_sine( double angle, double *sine, double *cosine)
{
   double power = 1.;
   size_t iter;

   angle -= (double)( (int)(angle / TWO_PI)) * TWO_PI;
   if( angle > PI)
      angle -= TWO_PI;
   else if( angle < -PI)
      angle += TWO_PI;
   *sine = *cosine = 0.;
   if( angle > PI / 2.)
      {
      angle -= PI;
      power = -1.;
      }
   else if( angle < -PI / 2.)
      {
      angle += PI;
      power = -1.;
      }
   for( iter = 1; iter < 10; power *= angle / (double)iter, iter++)
      switch( iter & 3)
         {
         case 1:
            *cosine += power;
            break;
         case 2:
            *sine += power;
            break;
         case 3:
            *cosine -= power;
            break;
         case 0:
            *sine -= power;
            break;
         }
}

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main( const int argc, const char **argv)
{
   int i;

   for( i = 1; i < argc; i++)
      {
      const double angle = atof( argv[i]) * PI / 180.;
      int16_t sine, cosine;
      const uint16_t iangle = (uint16_t)( angle * 32767. / PI);
      double dsin, dcos;

      cordic_sincos( iangle, &sine, &cosine);
      printf( "iangle %u sine %d cosine %d\n", iangle, sine, cosine);
      printf( "Analytic : %f %f\n", 16384. * sin( angle), 16384. * cos( angle));
      cosine_sine( angle, &dsin, &dcos);
      printf( "Power series : %f %f\n", dsin * 16384, dcos * 16384);
      }
   return( 0);
}

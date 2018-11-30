/* Two methods for computing the "exact" period of a pendulum,  i.e.,
not making the usual small-angle approximation.  This is from

http://leapsecond.com/hsn2006/pendulum-period-agm.pdf

   The paper over-states the case for the arithmetic-geometric mean
function.  It's not a "closed form" solution,  since the AGM cannot
be computed in closed form.  However,  it converges _much_ faster
than the traditional series solution,  especially for larger angles.
This also helps avoid accumulating roundoff errors.    */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

const double toler = 1e-16;

static double agm( double a, double b)
{
   int iter = 0;

   while( 1)
      {
      const double new_a = (a + b) * .5;
      const double new_b = sqrt( a * b);

      printf( "Iter %d : %.15f %.15f\n", iter++, new_a, new_b);
      if( fabs( new_a - new_b) >= fabs( a - b))
         return( new_a);
      a = new_a;
      b = new_b;
      }
}

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923

int main( const int argc, const char **argv)
{
   const double theta = atof( argv[1]) * PI / 180.;
   const double sine_term = sin( theta / 2.);
   const double sine_term2 = sine_term * sine_term;
   double series = 0., iterm = 1.;
   int i;

   assert( argc == 2);
   assert( theta > 0 && theta < PI);
   for( i = 2; iterm > toler; i += 2)
      {
      const double fraction = 1. - 1. / (double)i;

      series += iterm;
      iterm *= fraction * fraction * sine_term2;
      printf( "Term %d : %.15f\n", i / 2, iterm);
      }
   printf( "%.15f\n", series);
   printf( "%.15f\n", 1. / agm( 1., cos( theta / 2.)));
   return( 0);
}

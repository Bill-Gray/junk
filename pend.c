/* Two methods for computing the "exact" period of a pendulum,  i.e.,
not making the usual small-angle approximation.  This is from

http://leapsecond.com/hsn2006/pendulum-period-agm.pdf

   The paper over-states the case for the arithmetic-geometric mean
function.  It's not at all a "closed form" solution,  since the AGM
cannot be computed in closed form.  However,  it converges _much_ faster
than the traditional series solution,  especially for larger angles (see
table below).  This also helps avoid accumulating roundoff errors.

Angle      'usual'   AGM    Value
  10           7      3    1.0019071881
  20          10      4    1.0076690257
  30          13      4    1.0174087975
  40          16      5    1.0313405191
  50          10      5
  60          24      5
  70          30      5
  80          37      5
  90          46      5
 100          60      5
 110          79      5
 120         108      5
 130         156      6
 140         243      6
 150         428      6    1.7622037295
 160         943      6    2.0075074012
 170        3608      6    2.4393627196
 175       13741      7    2.8776635123
 177       36743      7    3.2021089248
 178       80115      7    3.4599710586
 179      303003      7    3.9010651603
 179.5   1142313      7    4.3422857879
 179.7   3030730      7    4.6674754565
 179.8   6565383      7    4.9255985167
 179.9  24530697      8    5.3668671090
 179.95 91224201      8    5.8081375943
 */

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
   int i, next_print = 2;

   assert( argc == 2);
   assert( theta > 0 && theta < PI);
   for( i = 2; iterm > toler; i += 2)
      {
      const double fraction = 1. - 1. / (double)i;

      series += iterm;
      iterm *= fraction * fraction * sine_term2;
      if( i >= next_print)
         {
         printf( "Term %d : %.15f\n", i / 2, iterm);
         next_print += next_print / 8 + 1;
         }
      }
   printf( "%.15f in %d terms\n", series, i / 2);
   printf( "%.15f\n", 1. / agm( 1., cos( theta / 2.)));
   return( 0);
}

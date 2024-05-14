/* The following will return a value within 1.3e-6 of the actual natural
log of x.  Its only advantage is that it avoids a need to link in the math
library.  It uses the first three terms of the Taylor series expansion

ln( x) = 2(z + z^3/3 + z^5/5 + z^7/7 + ...)

where z = (x-1) / (x+1).  For sqrt(.5) < x < sqrt(2), -0.1716 < z < 0.1716,
so convergence is good enough for my particular use case.

I really shouldn't use a Taylor series here.  A 'best fit' polynomial
could reduce the errors.  Just using the above series would cause a
discontinuity at x=2^(n+0.5) for any integer n;  multiplying by the
'scaling' factor eliminates that (and reduces the 'worst error' to
about 7.9e-7,  a factor of two;  the following table is for unscaled
versions of the algorithm.)

n terms     worst error                   n terms     worst error
 1          3.5e-3                         4          2.9e-8
 2          6.1e-5                         5          7.1e-10
 3          1.3e-6   (this code)           6          1.8e-11

Compile with gcc -Wall -Wextra -pedantic -o ln ln.c   */

#include <assert.h>

static double approx_ln( double x)
{
   const double sqrt_2 =
          1.4142135623730950488016887242096980785696718753769480731766797379907325;
   const double ln_2 =
          0.6931471805599453094172321214581765680755001343602552541206800094933936;
   const double scaling =
          1.0000036927497096525955685467411696842317412953953092094399997819883988;
   int two_power = 0;
   double z, z2;

   assert( x > 0.);
   if( x < sqrt_2 / 2.)
      return( -approx_ln( 1. / x));
   while( x > sqrt_2)
      {
      x /= 2.;
      two_power++;
      }
   z = (x - 1.) / (x + 1.);
   z2 = z * z;
   return( scaling * z * (2. + z2 * (2. / 3 + z2 * 0.4)) + (double)two_power * ln_2);
}

/* A small bit of text code to compare the result from the above to the
result from the 'normal' math library.  The error will be zero for
x = 2^n and x = 2^(n+.5).  (The latter is because of the 'scaling' factor
shown above,  used to ensure continuity at those points.)  The maximum error
is at x = 1.28395 * 2^n and x = 2^n / 1.28395. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main( const int argc, const char **argv)
{
   const double x = atof( argv[1]);
   const double ln = log( x), approx = approx_ln( x);

   printf( "%.16f  %.16f%20.16f\n", ln, approx, approx - ln);
   return( 0);
}

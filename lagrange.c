#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

/* Snippet to compute locations of the collinear Lagrangian points
(L1, L2, L3) relative to the primary and secondary.  This is found
by searching for zeroes of 'lagrange_func()',  (8.4.6) in J. M. A.
Danby's _Fundamentals of Orbital Mechanics_,  page 260.  See

https://en.wikipedia.org/wiki/Lagrange_point

for further numerical examples (though they're slightly in error;
compare to examples from this code shown at bottom.) This is used
in Find_Orb (see mpc_orb.cpp,  rovers.txt,  environ.def) to
compute locations of L1, L2, L3,  mostly so that ephemerides from
those positions can be given.  Compile with

gcc -Wall -Wextra -pedantic -o lagrange lagrange.c -lm

   This can be arranged to make a quintic polynomial,  and there
may actually be an analytic solution in this case.  I didn't try
very hard to find one.  Numerically finding the zeroes is quite
easy (secant method is used),  and I only do this once per
system,  then store the answer in 'environ.def'.    */

static double lagrange_func( const double mu, const double x)
{
   const double dx1 = x + mu;    /* x1 = -mu */
   const double dx2 = x - (1. - mu);   /* x2 = 1 - mu */

   return( x - (1 - mu) / (dx1 * fabs( dx1)) - mu / (dx2 * fabs( dx2)));
}

/* Zeroes of the above are found by the secant method.  The initial
estimate is pretty good (for L1,  x = 1 - cbrt(mu/3);  for L2,
x = 1 + cbrt( mu / 3);  for L3,  x = -1),  and convergence is quick
(usually five iterations or less). */

static int verbose = 0;

static double find_zero( const double mu, const int point)
{
   double x1, y1, x2, y2;
   size_t iter;

   if( point == 3)
      x1 = -1.;
   else
      x1 = 1. + (point == 1 ? -1. : 1.) * cbrt( mu / 3.);
   x1 -= mu;
   x2 = x1 + .001;
   y1 = lagrange_func( mu, x1);
   y2 = lagrange_func( mu, x2);
   for( iter = 15; x1 != x2 && y1 != y2 && iter; iter--)
      {
      double new_x = x1 + y1 * (x2 - x1) / (y1 - y2);

      if( verbose)
         printf( "%.15f   %.15f\n", x1 + mu, y1);
      if( point == 3 && new_x + mu > 0.)
         new_x = (x1 + mu) / 2. - mu;
      x2 = x1;
      y2 = y1;
      x1 = new_x;
      y1 = lagrange_func( mu, x1);
      }
   return( x1 + mu);
}

/* Example : mass(moon) / mass( earth + moon) = 0.0121505843786047316,
approximately.  Running

./lagrange 0.0121505843786047316, yields

0.84906572   1.16783275   -0.99291206

for the locations of Earth-Moon L1,  L2,  and L3 (i.e.,  L1 is 84.9066%
of the way to the moon from earth;  L2 is 16.7833% "beyond" the moon;
L3 is opposite the moon,  but only 99.2912% as far from us as the moon.)
Comparison to the Wikipedia page should make the meaning clear...  */

int main( const int argc, const char **argv)
{
   int i;
   bool is_secondary_to_primary_ratio = false;

   for( i = 1; i < argc; i++)
      if( argv[i][0] == '-' && argv[i][1] == 'v')
         verbose = 1;
      else if( argv[i][0] == '-' && argv[i][1] == 'r')
         is_secondary_to_primary_ratio = true;
      else
         {
         double mu = atof( argv[i]);

         if( is_secondary_to_primary_ratio)
            mu /= (1. + mu);
         printf( "%.15e  %.8f   %.8f   %.8f\n", mu,
              find_zero( mu, 1), find_zero( mu, 2), find_zero( mu, 3));
         }
   return( 0);
}

/* Results of the above code,  using masses from DE-438.

    mass2 / (mass1+mass2)   L1 ratio     L2 ratio      L3 ratio
Mer 1.660113877456495e-7   0.99619414   1.00381554   -0.99999990
Ven 2.447832295887156e-6   0.99068475   1.00937346   -0.99999857
EMB 3.040423403820060e-6   0.98998902   1.01007824   -0.99999823
Mar 3.227154996101724e-7   0.99525165   1.00476343   -0.99999981
Jup 9.538811521813125e-4   0.93331933   1.06978454   -0.99944357
Sat 2.858039654633085e-4   0.95503090   1.04635933   -0.99983328
Ura 4.366059029707305e-5   0.97578509   1.02461227   -0.99997453
Nep 5.151118418750341e-5   0.97442501   1.02601866   -0.99996995
Plu 7.350487779428069e-9   0.99865248   1.00134874   -1.00000000
*/

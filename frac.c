#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* References : http://www2.lowell.edu/rsch/LMI/etc_calc.php
(links to a paper describing underlying theory)  */

/* IGNORE THIS FILE;  it's served its purpose.  The following function
is basically a clone of the original 'expcalc' method for determining
the fraction of light that's within the specified aperture,  by doing
a numeric double integral.  I simply converted it from Tcl to C.

After that,  we have 'fraction_inside2',  which does the same thing,
but analytically.  This is (much,  much) faster and more accurate,  but
I needed to verify that I got the same result either way.  See 'expcalc.c'
for the actual use of the resulting function. */

static double fraction_inside( double fwhm, double radius, const double pixel_size)
{
   const int piece = 20;
   int max_pix_rad = 30;
   double psf_center_x = 0, psf_center_y = 0.;
// double psf_center_x = 0.5, psf_center_y = 0.5;
   double sigma2, radius2, rad_sum = 0., all_sum = 0.;
   const double bit = 1. / (double)piece;
   int i, j, k, l;

   fwhm /= pixel_size;
   radius /= pixel_size;
   if( radius > (double)max_pix_rad)
      {
      printf( "Warning : radius exceeds limit of %d\n", max_pix_rad);
      }
   sigma2 = fwhm / 2.35;
   sigma2 *= sigma2;
   radius2 = radius * radius;
   for( i = -max_pix_rad; i < max_pix_rad; i++)
      for( j = -max_pix_rad; j < max_pix_rad; j++)
         {
         double pix_sum = 0.;

         for( k = 0; k < piece; k++)
            {
            const double x = ((double)i - psf_center_x) + ((double)k + 0.5) * bit;
            const double fx = exp( -x * x  / (2. * sigma2));
            for( l = 0; l < piece; l++)
               {
               const double y = ((double)j - psf_center_y) + ((double)l + 0.5) * bit;
               const double fy = exp( -y * y  / (2. * sigma2));
               const double inten = fx * fy;
               const double this_bit = inten * bit * bit;

               pix_sum += this_bit;
               if( x * x + y * y < radius2)
                  rad_sum += this_bit;
               }
            }
         all_sum += pix_sum;
         }
   printf( "rad sum %f\n", rad_sum);
   printf( "all sum %f\n", all_sum);
   return( rad_sum / all_sum);
}

/* A Gaussian with a particular full-width-half-maximum (FWHM) 'fwhm'
will drop to half the maximum value at a radius fwhm/2.  Since the
intensity runs as I = I0 * exp( -(r/sigma)^2 / 2),  we can say :

I0/2 = I0 * exp( -(.5 * fwhm / sigma)^2 / 2)
1/2 = exp( -(.5 * fwhm / sigma)^2 / 2)
-ln(2) = -(.5 * fwhm / sigma)^2 / 2
2 * ln(2) = (.5 * fwhm / sigma)^2
fwhm / sigma = 2 * sqrt( 2 * ln(2)) = 2.354820045030949 */

static double fraction_inside2( double fwhm, const double radius)
{
   const full_widths_per_sigma = 2.354820045030949;
   const double r_scaled = full_widths_per_sigma * radius / fwhm;

   return( 1. - exp( -r_scaled * r_scaled / 2.));
}

static double effective_area( const double primary_diam, const double obstacle_diam)
{
   const double pi = 3.1415926535897932384626433832795028841971693993751058209749445923;

   return( pi * (primary_diam * primary_diam - obstacle_diam * obstacle_diam) / 4.);
}

/* NOTE that the corresponding function in the Tcl code gives a result per pixel */

static double sky_electrons_per_second_per_arcsec_squared(
            const double sky_brightness, const double collecting_area,
            const double n_photons)
{
   return( pow( 10., -0.4 * sky_brightness) * n_photons * collecting_area * qe);
}

int main( const int argc, const char **argv)
{
   const double fwhm = atof( argv[1]);
   const double radius = atof( argv[2]);
   const double pixel_size = atof( argv[3]);

   printf( "Fraction inside = %f\n",
      fraction_inside( fwhm, radius, pixel_size));
   printf( "Analytic result = %f\n",
      fraction_inside2( fwhm, radius));
   return( 0);
}

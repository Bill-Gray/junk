#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

/* A shameless clone of 'expcalc' by Frank Shelly,  leaning
heavily on his code.  Compile with :

gcc -Wall -Wextra -pedantic --std=c99 -o expcalc expcalc.c -lm

   References : http://www2.lowell.edu/rsch/LMI/etc_calc.php
(links to a paper describing underlying theory).  Note that the
formulae given in the paper for 'case 1' (compute exposure time
from SNR and magnitude) have the sign for B and C -- the linear
and constant coefficients of the quadratic to be solved --
backward;  both should be negated. */

typedef struct
{
   const char *mpc_code;
   char filter;
   double primary_diam, obstruction_diam;   /* in cm */
   double aperture, fwhm;                   /* arcsec */
   double qe;                               /* unitless,  0-1 */
   double readnoise;                        /* counts per pixel */
   double pixel_size;                       /* arcsec */
   double sky_brightness;                   /* magnitudes/arcsec^2 */
   double airmass;                          /* ~ 1/sin(alt) */
} expcalc_config_t;

typedef struct
{
   char band;
   double extinction;            /* mags per airmass */
   double zero_point;            /* counts from a mag 0 star in this band */
} filter_t;

const double pi = 3.1415926535897932384626433832795028841971693993751058209749445923;

/* A Gaussian with a particular full-width-half-maximum (FWHM) 'fwhm'
will drop to half the maximum value at a radius fwhm/2.  Since the
intensity runs as I = I0 * exp( -(r/sigma)^2 / 2),  we can say :

I0/2 = I0 * exp( -(.5 * fwhm / sigma)^2 / 2)
1/2 = exp( -(.5 * fwhm / sigma)^2 / 2)
-ln(2) = -(.5 * fwhm / sigma)^2 / 2
2 * ln(2) = (.5 * fwhm / sigma)^2
fwhm / sigma = 2 * sqrt( 2 * ln(2)) = 2.354820045030949 */

static double fraction_inside( const expcalc_config_t *c)
{
   const double full_widths_per_sigma = 2.354820045030949;
   const double r_scaled = full_widths_per_sigma * c->aperture / c->fwhm;

   return( 1. - exp( -r_scaled * r_scaled / 2.));
}

/* Sizes in centimeters */
static double effective_area( const expcalc_config_t *c)
{
   return( pi * (c->primary_diam * c->primary_diam - c->obstruction_diam * c->obstruction_diam) / 4.);
}

static double sky_electrons_per_second_per_pixel( const expcalc_config_t *c, const double zero_point)
{
   const double area = effective_area( c);
   const double sky_electrons_per_sec_per_square_arcsec =
                   pow( 10., -0.4 * c->sky_brightness) * zero_point * area * c->qe;

   return( sky_electrons_per_sec_per_square_arcsec * c->pixel_size * c->pixel_size);
}

static double star_electrons_per_second_per_pixel( const expcalc_config_t *c, const double mag,
               const filter_t *f)
{
   const double mag_corr = mag + c->airmass * f->extinction;
   const double rval = pow( 10., -0.4 * mag_corr) * f->zero_point * effective_area( c)
                                    * c->qe * fraction_inside( c);
   return( rval);
}

static double solve_quadratic( const double a, const double b, const double c)
{
   const double discr = b * b - 4. * a * c;

   assert( discr >= 0.);
   return( .5 * (sqrt( discr) - b) / a);
}

static int find_filter( const char filter, filter_t *f)
{
   size_t i;
   static filter_t filters[] = {
     { 'U', 0.60, 5.50e+05 },
     { 'B', 0.40, 3.91e+05 },
     { 'V', 0.20, 8.66e+05 },
     { 'R', 0.10, 1.10e+06 },
     { 'I', 0.08, 6.75e+05 },
     { 'N', 0.20, 4.32e+06 },
     { 'W', 0.15, 2.00e+06 } };

   for( i = 0; i < sizeof( filters) / sizeof( filters[0]); i++)
      if( filter == filters[i].band)
         {
         *f = filters[i];
         return( 0);
         }
   return( -1);
}

static int find_expcalc_config_from_mpc_code( const char *mpc_code, expcalc_config_t *c)
{
   int i = 0, rval;
   static expcalc_config_t configs[] = {
    /*   Code  Filt PrDi ObsDi Ape FWHM QE Rea  Pix   Sky  Airmass */
       { "I52", 'N', 100., 40., 6., 3., .9, 8., 1.036, 20., 1.5 },
       { "703", 'N',  72., 25., 6., 3., .9, 8., 3.,    20., 1.5 },
       { "G96", 'N', 152., 48., 6., 3., .9, 8., 1.5,   20., 1.5 },
       { "V06", 'N', 154., 40., 6., 3., .9, 8., 0.572, 20., 1.5 },
       { "E12", 'N',  50., 23., 6., 3., .9, 8., 1.8,   20., 1.5 },
       { NULL, ' ',    0.,  0., 0., 0., .0, 0., 0.0,    0., 0.0 } };

   while( configs[i].mpc_code && strcmp( configs[i].mpc_code, mpc_code))
      i++;
   if( configs[i].mpc_code)
      {
      *c = configs[i];
      rval = 0;
      }
   else              /* failed to find code */
      rval = -1;
   return( rval);
}

const char *usage_statement =
 "Usage:  /home/observer/bin/expcalc <switches>\n"
 "              SNR <exposure (sec)> <magnitude> |\n"
 "              mag <SNR> <exposure (sec)> |\n"
 "              exp <SNR> <magnitude>\n"
 " <switches>\n"
 "   -mpc <code>      # code to look up configuration values for (default I52)\n"
 "   -debug           # Output more verbose info to help with debugging\n"
 "   -filter <N|U|B|V|R|I|W>         # Change filter code N=none default N\n"
 "   -primary <diameter cm>          # Set primary aperature to this diameter in cm\n"
 "   -radius  <phot radius arcsc>    # Set pinhole photometry radius to this in arcsec\n"
 "   -qe <quantum efficiency>        # fractional quantum efficiency of the detector\n"
 "   -readnoise <electrons>          # detector readout noise in electrons\n"
 "   -pixelsize <arcsec>             # assume square pixel size in arcsec\n"
 "   -skybrightness <mag/arcsec/sec> # Magnitude of square arcsec sky in one second\n"
 "   -airmass <fraction>             # airmass of object\n"
 "   -fwhm <arcsec>                  # Full width half maximum in arcsec\n"
 "   -obstdiameter <cm>              # diameter in cm of central obstruction\n";

static void usage( void)
{
   fprintf( stderr, "%s", usage_statement);
   exit( -1);
}

/* Some test cases :

./expcalc -mpc V06 -primary 300 snr 60 22
SNR 4.54 with exposure time 60 seconds and magnitude 22.00

./expcalc snr 30 20.5
SNR 3.92 with exposure time 30 seconds and magnitude 20.50

./expcalc -skybrightness 19 -primary 200 -filter V snr 30 20.5
SNR 2.38 with exposure time 30 seconds and magnitude 20.50
*/

int main( const int argc, const char **argv)
{
   expcalc_config_t c;
   double mag = 0., exposure = 0., snr = 0.;
   double n_pixels_in_aperture, s, noise2, n_star;
   filter_t f;
   int i;
   const char *mpc_code = "I52";
   bool debug = false;

   for( i = 1; i < argc; i++)
      if( !strcmp( argv[i], "-mpc"))
          mpc_code = argv[i + 1];
   if( find_expcalc_config_from_mpc_code( mpc_code, &c))
      {
      fprintf( stderr, "Unrecognized MPC code '%s'\n", mpc_code);
      usage( );
      }
   for( i = 1; i < argc; i++)
      if( argv[i][0] == '-' && argv[i][1])
         {
         switch( argv[i][1])
            {
            case 'a':
               c.airmass = atof( argv[i + 1]);
               break;
            case 'd':
               debug = true;
               break;
            case 'f':
               if( argv[i][2] == 'i')
                  c.filter = argv[i + 1][0];
               if( argv[i][2] == 'w')
                  c.fwhm = atof( argv[i + 1]);
               break;
            case 'o':
               c.obstruction_diam = atof( argv[i + 1]);
               break;
            case 'p':
               if( argv[i][2] == 'r')     /* primary */
                  c.primary_diam = atof( argv[i + 1]);
               if( argv[i][2] == 'i')     /* pixel size */
                  c.pixel_size = atof( argv[i + 1]);
               break;
            case 'q':
               c.qe = atof( argv[i + 1]);
               break;
            case 'r':
               c.aperture = atof( argv[i + 1]);
               break;
            case 's':
               c.sky_brightness = atof( argv[i + 1]);
               break;
            }
         }
   for( i = 1; i < argc - 2; i++)
      if( !strcmp( argv[i], "snr") || !strcmp( argv[i], "SNR"))
         {
         exposure = atof( argv[i + 1]);
         mag = atof( argv[i + 2]);
         }
      else if( !strcmp( argv[i], "exp"))
         {
         snr = atof( argv[i + 1]);
         mag = atof( argv[i + 2]);
         }
      else if( !strcmp( argv[i], "mag"))
         {
         snr = atof( argv[i + 1]);
         exposure = atof( argv[i + 2]);
         }
   if( find_filter( c.filter, &f))
      {
      fprintf( stderr, "Unrecognized filter '%c'\n", c.filter);
      usage( );
      }

   if( debug)
      {
      printf( "%.2f-cm primary, %.2f-cm obstruction\n",
                  c.primary_diam, c.obstruction_diam);
      printf( "Filter %c, QE %.2f, read noise %.2f electrons/pixel\n",
                  c.filter, c.qe, c.readnoise);
      printf( "Pixels are %.2f arcsec;  aperture %.2f pixels, FWHM %.2f pixels\n",
                  c.pixel_size, c.aperture, c.fwhm);
      printf( "Sky brightness %.2f mag/arcsec^2; airmass %.2f\n",
                  c.sky_brightness, c.airmass);
      }

   n_pixels_in_aperture = pi * c.aperture * c.aperture / (c.pixel_size * c.pixel_size);
   s = sky_electrons_per_second_per_pixel( &c, f.zero_point);
   noise2 = c.readnoise * c.readnoise;
   if( mag)
      {
      n_star = star_electrons_per_second_per_pixel( &c, mag, &f);
      if( exposure)     /* got mag,  exposure,  need SNR */
         {
         const double signal = n_star * exposure;
         const double noise = sqrt( signal
                       + n_pixels_in_aperture * (s * exposure + noise2));

         snr = signal / noise;
         }
      else if( snr)     /* got mag,  SNR,  computing exposure */
         exposure = solve_quadratic( n_star * n_star,
                        -snr * snr * (n_star + n_pixels_in_aperture * s),
                        -snr * snr * n_pixels_in_aperture * noise2);
      else
         usage( );
      }
   else if( snr && exposure)      /* computing mag */
      {
      double mag_exp;
      const double tval = snr * snr * exposure;

      n_star = solve_quadratic( exposure * exposure, -tval,
                     n_pixels_in_aperture * (noise2 - tval * s));
      mag_exp = n_star / (f.zero_point * effective_area( &c) * c.qe * fraction_inside( &c));
      mag = -2.5 * log10( mag_exp);
      mag -=  c.airmass * f.extinction;
      }
   else
      usage( );
   if( debug)
      {
      printf( "Sky brightness (counts/pixel/s) = %.1f\n", s);
      printf( "n_star = %.1f\n", n_star);
      }
   printf( "SNR %.2f with exposure time %.0f seconds and magnitude %.2f\n",
                  snr, exposure, mag);
   return( 0);
}

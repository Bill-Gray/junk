#include <stdio.h>
#include <assert.h>
#include <math.h>

/* See also ata.txt.  This takes a list of telescope positions for
the Allen Telescope Array and converts from north/east/delta-alt
values to lat/lon/alt values.  The result is written to the console
in the "rovers.txt" format,  suitable for use in Find_Orb.  The results
line up visually with G__gle Earth.  But I can't really check altitude,
and there's a comment in ata.txt about "rotated 1deg" which I find
unnerving.  Were it rotated a degree around Z,  we'd see the result
on G__gle Earth;  plotted lat/lons wouldn't match those seen on the map.
But a rotation around an axis horizontal to the earth would be
impossible to see (but would cause trouble.)

Compile with

gcc -Wall -Wextra -pedantic -o ata ata.c -lm
*/

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923
#define EARTH_RADIUS 6378140.

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

/* Use a small angle approximation to do the conversion.  The telescopes
are not spread out enough,  nor do they make sufficiently precise
measurements,  for us to see the difference.  I took a lat/lon for ATA
from Wikipaedia and was off by a couple of hundred meters.  I then
measured AT03 on G__gle Maps and made the correction shown in the last
two lines.        */

static void cvt_to_latlon( double *lat, double *lon, const double x, const double y,
               const double z)
{
   const double lat0 = +40.8178 * PI / 180.;
   const double lon0 = -121.4695 * PI / 180.;         /* Wikipaedia */

   INTENTIONALLY_UNUSED_PARAMETER( z);
   *lat = lat0 + y / EARTH_RADIUS;
   *lon = lon0 + x / (EARTH_RADIUS * cos( lat0));
   *lat += (40.81595 - 40.81640418) * PI / 180.;
   *lon += (-121.47072 + 121.46840201) * PI / 180.;
}

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( "ata.txt", "rb");
   char buff[200];

   INTENTIONALLY_UNUSED_PARAMETER( argc);
   INTENTIONALLY_UNUSED_PARAMETER( argv);
   assert( ifile);
   while( fgets( buff, sizeof( buff), ifile))
      if( *buff != '#')
         {
         double x, y, z, lat, lon;
         int n_scanned = sscanf( buff, "%lf %lf %lf", &y, &x, &z);
         const double z0 = 986.123;    /* rough altitude,  from Wi____dia */

         assert( n_scanned == 3);
         cvt_to_latlon( &lat, &lon, x, y, z);
         z += z0;
         printf( "AT%.2s!%13.8f   %+12.8f   %8.3f   ATA %.2s\n",
                     buff + 40, lon * 180. / PI, lat * 180. / PI, z, buff + 40);
         }
   fclose( ifile);
   return( 0);
}


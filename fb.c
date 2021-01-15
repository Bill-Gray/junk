#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/fb.h>
#include <assert.h>

#define INTENTIONALLY_UNUSED_PARAMETER( param) (void)(param)

int main( const int argc, const char **argv)
{
   int fb_fd = open( "/dev/fb0", O_RDWR);
   int i, j;
   struct fb_fix_screeninfo finfo;
   struct fb_var_screeninfo vinfo;
   int err;
   uint8_t *buff;

   INTENTIONALLY_UNUSED_PARAMETER( argc);
   INTENTIONALLY_UNUSED_PARAMETER( argv);
   if( fb_fd <= 0)
      perror( "fb_fd <= 0");
   assert( fb_fd > 0);

   /* Get variable screen information */
   err = ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
   assert( !err);
   printf( "Resolution: %u x %u\n", (unsigned)vinfo.xres, (unsigned)vinfo.yres);
   printf( "Virt Res: %u x %u\n",
                    (unsigned)vinfo.xres_virtual, (unsigned)vinfo.yres_virtual);
   printf( "%u bits/pixel\n", (unsigned)vinfo.bits_per_pixel);
   printf( "Grayscale: %u\n", (unsigned)vinfo.grayscale);

   /*  Get fixed screen information */
   err = ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
   assert( !err);
   printf( "ID: '%s'\n", finfo.id);
   printf( "%u bytes/line\n", (unsigned)finfo.line_length);
   printf( "%u bytes memory needed\n", (unsigned)finfo.smem_len);

   buff = mmap( NULL, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
   assert( buff != MAP_FAILED);

   if ( vinfo.bits_per_pixel == 8)        /* 256 color palette */
      {                              /* leave first 16 colors at default; */
      uint16_t r[256], g[256], b[256];   /* set others to gray scales */
      struct fb_cmap pal;

      for( i = 0; i < 256; i++)
         r[i] = g[i] = b[i] = (uint16_t)( i << 8);
      pal.start = 16;
      pal.len = 240;
      pal.red = r;
      pal.green = g;
      pal.blue = b;
      pal.transp = NULL;
      if( ioctl( fb_fd, FBIOPUTCMAP, &pal))
         printf( "Error setting palette.\n");
      }

   for( i = 300; i < 500; i++)
      if( vinfo.bits_per_pixel == 32)        /* full 32-bit color */
         {
         uint32_t *tptr = (uint32_t *)( buff + i * finfo.line_length);

         for( j = 0; j < 255; j++)
            tptr[j + 200] = 0xff0000 + j + ((i - 300) << 8);
         }
      else if( vinfo.bits_per_pixel == 8)        /* 256 color palette */
         {
         uint8_t *tptr = buff + i * finfo.line_length;

         for( j = 0; j < 255; j++)
            tptr[j + 200] = (uint8_t)j;
         }
   getchar( );
   munmap( buff, finfo.smem_len);
   close( fb_fd);
   if( vinfo.bits_per_pixel != 8 && vinfo.bits_per_pixel != 32)
      printf( "At least at present,  this program doesn't support this bit depth.\n");
   return( 0);
}

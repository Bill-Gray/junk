/* Copyright (C) 2018  C. McEnroe <june@causal.agency>
 *
 * Code came from https://cmcenroe.me/2018/01/30/fbclock.html
 * https://github.com/causal-agent/src/blob/bd4afa842d5893a54ba9525bb7c9a2c691ec6325/bin/fbclock.c
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Some useful info on setting/getting palette entries,  and some other
framebuffer issues,  is at

http://raspberrycompote.blogspot.com/2013/03/low-level-graphics-on-raspberry-pi-part_7.html
*/


#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>

static const uint32_t PSF2Magic = 0x864AB572;
struct PSF2Header {
   uint32_t magic;
   uint32_t version;
   uint32_t headerSize;
   uint32_t flags;
   uint32_t glyphCount;
   uint32_t glyphSize;
   uint32_t glyphHeight;
   uint32_t glyphWidth;
};

int main() {
   uint8_t *glyphs;
   struct PSF2Header header;
   size_t len;
   uint8_t *buf;
   struct fb_fix_screeninfo finfo;
   struct fb_var_screeninfo vinfo;
   gzFile font = NULL;
   const char *fontPath = getenv("FONT");
   const char *fbPath = getenv("FRAMEBUFFER");
   int fb, error;

   if( fontPath && *fontPath)
      font = gzopen( fontPath, "r");
   if( !font)
      font = gzopen( "/usr/share/consolefonts/Lat2-TerminusBold20x10.psf.gz", "r");
   if( !font)
      font = gzopen( "/usr/share/kbd/consolefonts/Lat2-Terminus16.psfu.gz", "r");

   if (!font) err(EX_NOINPUT, "Font not loaded");

   len = gzread(font, &header, sizeof(header));
   if (!len && gzeof(font)) errx(EX_DATAERR, "%s: missing header", fontPath);
   if (!len) errx(EX_IOERR, "%s", gzerror(font, NULL));

   if (header.magic != PSF2Magic) {
      errx(
         EX_DATAERR, "%s: invalid header magic %08X",
         fontPath, header.magic
      );
   }
   if (header.headerSize != sizeof(struct PSF2Header)) {
      errx(
         EX_DATAERR, "%s: weird header size %d",
         fontPath, header.headerSize
      );
   }

   glyphs = (uint8_t *)malloc( 128 * header.glyphSize);
   assert( glyphs);
   len = gzread( font, glyphs, header.glyphSize * 128);
   if (!len && gzeof(font)) errx(EX_DATAERR, "%s: missing glyphs", fontPath);
   if (!len) errx(EX_IOERR, "%s", gzerror(font, NULL));

   gzclose(font);

   if (!fbPath) fbPath = "/dev/fb0";

   fb = open(fbPath, O_RDWR);
   if (fb < 0) err(EX_OSFILE, "%s", fbPath);

   error = ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
   if (error) err(EX_IOERR, "%s", fbPath);
   /*  Get fixed screen information */
   error = ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
   if (error) err(EX_IOERR, "%s", fbPath);

   buf = mmap(NULL, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
   if (buf == MAP_FAILED) err(EX_IOERR, "%s", fbPath);

   for (;;) {
      const uint32_t DarkWhite = 0xffffff;
      const uint32_t DarkBlack = 0x0;
      uint32_t left, bottom = header.glyphHeight;
      time_t t = time(NULL);
      char str[64];
      const struct tm *local = localtime(&t);

      if (t < 0) err(EX_OSERR, "time");
      if (!local) err(EX_OSERR, "localtime");

      len = strftime(str, sizeof(str), "%H:%M:%S", local);
      assert(len);

      left = vinfo.xres - header.glyphWidth * len;
      if( vinfo.bits_per_pixel == 32) {
         uint32_t *tptr = (uint32_t *)buf;
         uint32_t x, y;
         const int line_len = finfo.line_length / sizeof( uint32_t);
         const char *s;

         for( y = 0; y < bottom; ++y) {
            tptr[y * line_len + left - 1] = DarkWhite;
         }
         for( x = left - 1; x < vinfo.xres; ++x) {
            tptr[bottom * line_len + x] = DarkWhite;
         }

         for( s = str; *s; ++s) {
            const uint8_t *glyph = glyphs + (unsigned)*s * header.glyphSize;
            uint32_t stride = header.glyphSize / header.glyphHeight;

            for( y = 0; y < header.glyphHeight; ++y) {
               for( x = 0; x < header.glyphWidth; ++x) {
                  uint8_t bits = glyph[y * stride + x / 8];
                  uint8_t bit = bits >> (7 - x % 8) & 1;
                  tptr[y * line_len + left + x] = bit
                     ? DarkWhite
                     : DarkBlack;
               }
            }
            left += header.glyphWidth;
         }
      }

      else if( vinfo.bits_per_pixel == 8) {
         uint32_t x, y;
         const char *s;

         for( y = 0; y < bottom; ++y) {
            buf[y * finfo.line_length + left - 1] = (uint8_t)7;
         }
         for( x = left - 1; x < vinfo.xres; ++x) {
            buf[bottom * finfo.line_length + x] = (uint8_t)7;
         }

         for( s = str; *s; ++s) {
            const uint8_t *glyph = glyphs + (unsigned)*s * header.glyphSize;
            uint32_t stride = header.glyphSize / header.glyphHeight;
            for( y = 0; y < header.glyphHeight; ++y) {
               for( x = 0; x < header.glyphWidth; ++x) {
                  uint8_t bits = glyph[y * stride + x / 8];
                  uint8_t bit = bits >> (7 - x % 8) & 1;
                  buf[y * finfo.line_length + left + x] = bit
                     ? 7 : 0;
               }
            }
            left += header.glyphWidth;
         }
      }
      else {
         fprintf( stderr, "%d bits/pixel isn't supported\n", vinfo.bits_per_pixel);
         return( 0);
      }
      sleep( 1);
   }
   free( glyphs);
}

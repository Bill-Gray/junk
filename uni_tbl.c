#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* Code to generate a table of intervals of Unicode points for zero-width
character (mostly combining characters) or of full-width characters.  These
are for use with Markus Kuhn's implementations of wcwidth() and wcswidth():

https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c

(which is excellent except it only goes up to Unicode 5.0;  hence my
writing this bit of code.)  This is also for use in PDCursesMod (see
pdcurses/addch.c).  This outputs C arrays suitable for nearly direct
inclusion in the code.  It uses the data from

https://unicode.org/Public/UNIDATA/EastAsianWidth.txt

and requires that EastAsianWidth.txt be in the working directory.  Run
with no command line arguments to get a table of zero-width points,  or
with any command line argument to get a table of full-width points.

Markus Kuhn's original implementation used 'uniset' to generate the arrays :

https://github.com/depp/uniset

which I was unable to get to work.  However,  I used the same output (by
default,  the 16-bit array scheme) and the code provided in the README.md
for that project for 'uniset_test()' will work with the arrays generated
by this code.

Note that the tables generated (also shown at the bottom) are used in
'addch.c' in PDCursesMod (q.v.) on platforms that lack a built-in wcwidth().
*/

static int is_combining_range( const char *buff)
{
   if( strlen( buff) > 24 && *buff != '#')
      if( !memcmp( buff + 17, "# Me ", 5) || !memcmp( buff + 17, "# Mn ", 5)
                                          || !memcmp( buff + 17, "# Cf ", 5)
                                          || !memcmp( buff, "1160", 4))
         return( 1);

   return( 0);
}

static int is_wide( const char *buff)
{
   if( strlen( buff) > 24 && *buff != '#')
      if( strstr( buff, ";W ") || strstr( buff, ";F "))
         return( 1);

   return( 0);
}

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( "EastAsianWidth.txt", "rb");
   char buff[300];
   unsigned low = 0, high = 0, n_output = 0, i;
   unsigned n_allocated = 16;
   unsigned *output = (unsigned *)calloc( n_allocated, sizeof( unsigned));
   int sixteen_bit = 1, output_wide = 0;

   for( i = 1; i < (unsigned)argc; i++)
      if( !strcmp( argv[i], "--32"))
         sixteen_bit = 0;
      else if( !strcmp( argv[i], "-w"))
         output_wide = 1;
      else
         fprintf( stderr, "Didn't understand argument '%s'\n", argv[i]);

   if( !ifile)
      {
      fprintf( stderr, "Didn't open 'EastAsianWidth.txt'.  This file can be\n"
            "found at https://github.com/depp/uniset.\n");
      return( -1);
      }
   while( fgets( buff, sizeof( buff), ifile))
      if( output_wide ? is_wide( buff) : is_combining_range( buff))
         {
         unsigned int low1 = 0, high1 = 0, n_found;

         n_found = sscanf( buff, "%x..%x", &low1, &high1);
         if( n_found == 1)
            high1 = low1;
         else if( n_found != 2)
            {
            fprintf( stderr, "Error\n%s\n", buff);
            return( -1);
            }
         if( !low)         /* new range */
            low = low1;
         high = high1;
         }
      else if( high > 0xad)
         {
         if( n_output >= n_allocated - 8)
            {
            n_allocated <<= 1;
            output = (unsigned *)realloc( output, n_allocated * sizeof( unsigned));
            }
         output[n_output++] = low;
         if( sixteen_bit)
            while( (low ^ high) & 0xff0000)        /* i.e.,  they span a plane */
               {
               low = (low & 0xff0000) + 0x10000;
               output[n_output++] = low - 1;
               output[n_output++] = low;
               }
         output[n_output++] = high;
         low = high = 0;
         }
   fclose( ifile);
   if( high > 0xad)
      {
      output[n_output++] = low;
      output[n_output++] = high;
      }
   printf( "const uint%d_t uniset_table[][2] = {\n",
               (sixteen_bit ? 16 : 32));
   if( sixteen_bit)
      {
      unsigned idx_low[17], idx_high[17];

      for( i = 0; i < 17; i++)
         idx_low[i] = idx_high[i] = 0;

      for( i = 0; i < n_output; i += 2)
         {
         unsigned j = output[i] >> 16;

         if( !idx_high[j])
            idx_low[j] = i / 2;
         idx_high[j] = i / 2 + 1;
         }
      for( i = 0; i < 17; i++)
         printf( " { /* plane %d */ %d, %d },\n", i,
                  idx_low[i], idx_high[i]);
      }
   for( i = 0; i < n_output; i += 2)
      {
      const unsigned mask = (sixteen_bit ? 0xffff : 0xffffff);

      printf( " { 0x%04X, 0x%04X }", output[i] & mask, output[i + 1] & mask);
      if( i == n_output - 2)
         printf( " };\n");
      else
         printf( (i % 6 == 4 ? ",\n" : ","));
      }
   free( output);
   return( 0);
}

#ifdef EXAMPLE_OUTPUT

/* Tables generated with the EastAsianWidth-14.0.0.txt version : */

const uint16_t tbl_for_zero_width_chars[][2] = {
 { /* plane 0 */ 0, 202 },
 { /* plane 1 */ 202, 312 },
 { /* plane 2 */ 0, 0 },
 { /* plane 3 */ 0, 0 },
 { /* plane 4 */ 0, 0 },
 { /* plane 5 */ 0, 0 },
 { /* plane 6 */ 0, 0 },
 { /* plane 7 */ 0, 0 },
 { /* plane 8 */ 0, 0 },
 { /* plane 9 */ 0, 0 },
 { /* plane 10 */ 0, 0 },
 { /* plane 11 */ 0, 0 },
 { /* plane 12 */ 0, 0 },
 { /* plane 13 */ 0, 0 },
 { /* plane 14 */ 312, 313 },
 { /* plane 15 */ 0, 0 },
 { /* plane 16 */ 0, 0 },
 { 0x00AD, 0x036F }, { 0x0483, 0x0489 }, { 0x0591, 0x05BD },
 { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 }, { 0x05C4, 0x05C5 },
 { 0x05C7, 0x05C7 }, { 0x0600, 0x0605 }, { 0x0610, 0x061A },
 { 0x061C, 0x061C }, { 0x064B, 0x065F }, { 0x0670, 0x0670 },
 { 0x06D6, 0x06DD }, { 0x06DF, 0x06E4 }, { 0x06E7, 0x06E8 },
 { 0x06EA, 0x06ED }, { 0x070F, 0x070F }, { 0x0711, 0x0711 },
 { 0x0730, 0x074A }, { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 },
 { 0x07FD, 0x07FD }, { 0x0816, 0x0819 }, { 0x081B, 0x0823 },
 { 0x0825, 0x0827 }, { 0x0829, 0x082D }, { 0x0859, 0x085B },
 { 0x0890, 0x089F }, { 0x08CA, 0x0902 }, { 0x093A, 0x093A },
 { 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
 { 0x0951, 0x0957 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
 { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
 { 0x09E2, 0x09E3 }, { 0x09FE, 0x0A02 }, { 0x0A3C, 0x0A3C },
 { 0x0A41, 0x0A51 }, { 0x0A70, 0x0A71 }, { 0x0A75, 0x0A75 },
 { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC }, { 0x0AC1, 0x0AC8 },
 { 0x0ACD, 0x0ACD }, { 0x0AE2, 0x0AE3 }, { 0x0AFA, 0x0B01 },
 { 0x0B3C, 0x0B3C }, { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B44 },
 { 0x0B4D, 0x0B56 }, { 0x0B62, 0x0B63 }, { 0x0B82, 0x0B82 },
 { 0x0BC0, 0x0BC0 }, { 0x0BCD, 0x0BCD }, { 0x0C00, 0x0C00 },
 { 0x0C04, 0x0C04 }, { 0x0C3C, 0x0C3C }, { 0x0C3E, 0x0C40 },
 { 0x0C46, 0x0C56 }, { 0x0C62, 0x0C63 }, { 0x0C81, 0x0C81 },
 { 0x0CBC, 0x0CBC }, { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 },
 { 0x0CCC, 0x0CCD }, { 0x0CE2, 0x0CE3 }, { 0x0D00, 0x0D01 },
 { 0x0D3B, 0x0D3C }, { 0x0D41, 0x0D44 }, { 0x0D4D, 0x0D4D },
 { 0x0D62, 0x0D63 }, { 0x0D81, 0x0D81 }, { 0x0DCA, 0x0DCA },
 { 0x0DD2, 0x0DD6 }, { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A },
 { 0x0E47, 0x0E4E }, { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EBC },
 { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
 { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
 { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F8D, 0x0FBC },
 { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 }, { 0x1032, 0x1037 },
 { 0x1039, 0x103A }, { 0x103D, 0x103E }, { 0x1058, 0x1059 },
 { 0x105E, 0x1060 }, { 0x1071, 0x1074 }, { 0x1082, 0x1082 },
 { 0x1085, 0x1086 }, { 0x108D, 0x108D }, { 0x109D, 0x109D },
 { 0x1160, 0x11FF }, { 0x135D, 0x135F }, { 0x1712, 0x1714 },
 { 0x1732, 0x1733 }, { 0x1752, 0x1753 }, { 0x1772, 0x1773 },
 { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD }, { 0x17C6, 0x17C6 },
 { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD }, { 0x180B, 0x180F },
 { 0x1885, 0x1886 }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
 { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
 { 0x1A17, 0x1A18 }, { 0x1A1B, 0x1A1B }, { 0x1A56, 0x1A56 },
 { 0x1A58, 0x1A60 }, { 0x1A62, 0x1A62 }, { 0x1A65, 0x1A6C },
 { 0x1A73, 0x1A7F }, { 0x1AB0, 0x1B03 }, { 0x1B34, 0x1B34 },
 { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
 { 0x1B6B, 0x1B73 }, { 0x1B80, 0x1B81 }, { 0x1BA2, 0x1BA5 },
 { 0x1BA8, 0x1BA9 }, { 0x1BAB, 0x1BAD }, { 0x1BE6, 0x1BE6 },
 { 0x1BE8, 0x1BE9 }, { 0x1BED, 0x1BED }, { 0x1BEF, 0x1BF1 },
 { 0x1C2C, 0x1C33 }, { 0x1C36, 0x1C37 }, { 0x1CD0, 0x1CD2 },
 { 0x1CD4, 0x1CE0 }, { 0x1CE2, 0x1CE8 }, { 0x1CED, 0x1CED },
 { 0x1CF4, 0x1CF4 }, { 0x1CF8, 0x1CF9 }, { 0x1DC0, 0x1DFF },
 { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x206F },
 { 0x20D0, 0x20F0 }, { 0x2CEF, 0x2CF1 }, { 0x2D7F, 0x2D7F },
 { 0x2DE0, 0x2DFF }, { 0x302A, 0x302D }, { 0x3099, 0x309A },
 { 0xA66F, 0xA672 }, { 0xA674, 0xA67D }, { 0xA69E, 0xA69F },
 { 0xA6F0, 0xA6F1 }, { 0xA802, 0xA802 }, { 0xA806, 0xA806 },
 { 0xA80B, 0xA80B }, { 0xA825, 0xA826 }, { 0xA82C, 0xA82C },
 { 0xA8C4, 0xA8C5 }, { 0xA8E0, 0xA8F1 }, { 0xA8FF, 0xA8FF },
 { 0xA926, 0xA92D }, { 0xA947, 0xA951 }, { 0xA980, 0xA982 },
 { 0xA9B3, 0xA9B3 }, { 0xA9B6, 0xA9B9 }, { 0xA9BC, 0xA9BD },
 { 0xA9E5, 0xA9E5 }, { 0xAA29, 0xAA2E }, { 0xAA31, 0xAA32 },
 { 0xAA35, 0xAA36 }, { 0xAA43, 0xAA43 }, { 0xAA4C, 0xAA4C },
 { 0xAA7C, 0xAA7C }, { 0xAAB0, 0xAAB0 }, { 0xAAB2, 0xAAB4 },
 { 0xAAB7, 0xAAB8 }, { 0xAABE, 0xAABF }, { 0xAAC1, 0xAAC1 },
 { 0xAAEC, 0xAAED }, { 0xAAF6, 0xAAF6 }, { 0xABE5, 0xABE5 },
 { 0xABE8, 0xABE8 }, { 0xABED, 0xABED }, { 0xFB1E, 0xFB1E },
 { 0xFE00, 0xFE0F }, { 0xFE20, 0xFE2F }, { 0xFEFF, 0xFEFF },
 { 0xFFF9, 0xFFFB }, { 0x01FD, 0x01FD }, { 0x02E0, 0x02E0 },
 { 0x0376, 0x037A }, { 0x0A01, 0x0A0F }, { 0x0A38, 0x0A3F },
 { 0x0AE5, 0x0AE6 }, { 0x0D24, 0x0D27 }, { 0x0EAB, 0x0EAC },
 { 0x0F46, 0x0F50 }, { 0x0F82, 0x0F85 }, { 0x1001, 0x1001 },
 { 0x1038, 0x1046 }, { 0x1070, 0x1070 }, { 0x1073, 0x1074 },
 { 0x107F, 0x1081 }, { 0x10B3, 0x10B6 }, { 0x10B9, 0x10BA },
 { 0x10BD, 0x10BD }, { 0x10C2, 0x10CD }, { 0x1100, 0x1102 },
 { 0x1127, 0x112B }, { 0x112D, 0x1134 }, { 0x1173, 0x1173 },
 { 0x1180, 0x1181 }, { 0x11B6, 0x11BE }, { 0x11C9, 0x11CC },
 { 0x11CF, 0x11CF }, { 0x122F, 0x1231 }, { 0x1234, 0x1234 },
 { 0x1236, 0x1237 }, { 0x123E, 0x123E }, { 0x12DF, 0x12DF },
 { 0x12E3, 0x12EA }, { 0x1300, 0x1301 }, { 0x133B, 0x133C },
 { 0x1340, 0x1340 }, { 0x1366, 0x1374 }, { 0x1438, 0x143F },
 { 0x1442, 0x1444 }, { 0x1446, 0x1446 }, { 0x145E, 0x145E },
 { 0x14B3, 0x14B8 }, { 0x14BA, 0x14BA }, { 0x14BF, 0x14C0 },
 { 0x14C2, 0x14C3 }, { 0x15B2, 0x15B5 }, { 0x15BC, 0x15BD },
 { 0x15BF, 0x15C0 }, { 0x15DC, 0x162F }, { 0x1633, 0x163A },
 { 0x163D, 0x163D }, { 0x163F, 0x1640 }, { 0x16AB, 0x16AB },
 { 0x16AD, 0x16AD }, { 0x16B0, 0x16B5 }, { 0x16B7, 0x16B7 },
 { 0x171D, 0x171F }, { 0x1722, 0x1725 }, { 0x1727, 0x172B },
 { 0x182F, 0x1837 }, { 0x1839, 0x183A }, { 0x193B, 0x193C },
 { 0x193E, 0x193E }, { 0x1943, 0x1943 }, { 0x19D4, 0x19DB },
 { 0x19E0, 0x19E0 }, { 0x1A01, 0x1A0A }, { 0x1A33, 0x1A38 },
 { 0x1A3B, 0x1A3E }, { 0x1A47, 0x1A47 }, { 0x1A51, 0x1A56 },
 { 0x1A59, 0x1A5B }, { 0x1A8A, 0x1A96 }, { 0x1A98, 0x1A99 },
 { 0x1C30, 0x1C3D }, { 0x1C3F, 0x1C3F }, { 0x1C92, 0x1CA7 },
 { 0x1CAA, 0x1CB0 }, { 0x1CB2, 0x1CB3 }, { 0x1CB5, 0x1CB6 },
 { 0x1D31, 0x1D45 }, { 0x1D47, 0x1D47 }, { 0x1D90, 0x1D91 },
 { 0x1D95, 0x1D95 }, { 0x1D97, 0x1D97 }, { 0x1EF3, 0x1EF4 },
 { 0x3430, 0x3438 }, { 0x6AF0, 0x6AF4 }, { 0x6B30, 0x6B36 },
 { 0x6F4F, 0x6F4F }, { 0x6F8F, 0x6F92 }, { 0x6FE4, 0x6FE4 },
 { 0xBC9D, 0xBC9E }, { 0xBCA0, 0xCF46 }, { 0xD167, 0xD169 },
 { 0xD173, 0xD182 }, { 0xD185, 0xD18B }, { 0xD1AA, 0xD1AD },
 { 0xD242, 0xD244 }, { 0xDA00, 0xDA36 }, { 0xDA3B, 0xDA6C },
 { 0xDA75, 0xDA75 }, { 0xDA84, 0xDA84 }, { 0xDA9B, 0xDAAF },
 { 0xE000, 0xE02A }, { 0xE130, 0xE136 }, { 0xE2AE, 0xE2AE },
 { 0xE2EC, 0xE2EF }, { 0xE8D0, 0xE8D6 }, { 0xE944, 0xE94A },
 { 0x0001, 0x01EF } };

const uint16_t tbl_for_fullwidth_chars[][2] = {
 { /* plane 0 */ 0, 46 },
 { /* plane 1 */ 46, 80 },
 { /* plane 2 */ 80, 81 },
 { /* plane 3 */ 81, 82 },
 { /* plane 4 */ 0, 0 },
 { /* plane 5 */ 0, 0 },
 { /* plane 6 */ 0, 0 },
 { /* plane 7 */ 0, 0 },
 { /* plane 8 */ 0, 0 },
 { /* plane 9 */ 0, 0 },
 { /* plane 10 */ 0, 0 },
 { /* plane 11 */ 0, 0 },
 { /* plane 12 */ 0, 0 },
 { /* plane 13 */ 0, 0 },
 { /* plane 14 */ 0, 0 },
 { /* plane 15 */ 0, 0 },
 { /* plane 16 */ 0, 0 },
 { 0x1100, 0x115F }, { 0x231A, 0x231B }, { 0x2329, 0x232A },
 { 0x23E9, 0x23EC }, { 0x23F0, 0x23F0 }, { 0x23F3, 0x23F3 },
 { 0x25FD, 0x25FE }, { 0x2614, 0x2615 }, { 0x2648, 0x2653 },
 { 0x267F, 0x267F }, { 0x2693, 0x2693 }, { 0x26A1, 0x26A1 },
 { 0x26AA, 0x26AB }, { 0x26BD, 0x26BE }, { 0x26C4, 0x26C5 },
 { 0x26CE, 0x26CE }, { 0x26D4, 0x26D4 }, { 0x26EA, 0x26EA },
 { 0x26F2, 0x26F3 }, { 0x26F5, 0x26F5 }, { 0x26FA, 0x26FA },
 { 0x26FD, 0x26FD }, { 0x2705, 0x2705 }, { 0x270A, 0x270B },
 { 0x2728, 0x2728 }, { 0x274C, 0x274C }, { 0x274E, 0x274E },
 { 0x2753, 0x2755 }, { 0x2757, 0x2757 }, { 0x2795, 0x2797 },
 { 0x27B0, 0x27B0 }, { 0x27BF, 0x27BF }, { 0x2B1B, 0x2B1C },
 { 0x2B50, 0x2B50 }, { 0x2B55, 0x2B55 }, { 0x2E80, 0x303E },
 { 0x3041, 0x3247 }, { 0x3250, 0x4DBF }, { 0x4E00, 0xA4C6 },
 { 0xA960, 0xA97C }, { 0xAC00, 0xD7A3 }, { 0xF900, 0xFAFF },
 { 0xFE10, 0xFE19 }, { 0xFE30, 0xFE6B }, { 0xFF01, 0xFF60 },
 { 0xFFE0, 0xFFE6 }, { 0x6FE0, 0xB2FB }, { 0xF004, 0xF004 },
 { 0xF0CF, 0xF0CF }, { 0xF18E, 0xF18E }, { 0xF191, 0xF19A },
 { 0xF200, 0xF320 }, { 0xF32D, 0xF335 }, { 0xF337, 0xF37C },
 { 0xF37E, 0xF393 }, { 0xF3A0, 0xF3CA }, { 0xF3CF, 0xF3D3 },
 { 0xF3E0, 0xF3F0 }, { 0xF3F4, 0xF3F4 }, { 0xF3F8, 0xF43E },
 { 0xF440, 0xF440 }, { 0xF442, 0xF4FC }, { 0xF4FF, 0xF53D },
 { 0xF54B, 0xF54E }, { 0xF550, 0xF567 }, { 0xF57A, 0xF57A },
 { 0xF595, 0xF596 }, { 0xF5A4, 0xF5A4 }, { 0xF5FB, 0xF64F },
 { 0xF680, 0xF6C5 }, { 0xF6CC, 0xF6CC }, { 0xF6D0, 0xF6D2 },
 { 0xF6D5, 0xF6DF }, { 0xF6EB, 0xF6EC }, { 0xF6F4, 0xF6FC },
 { 0xF7E0, 0xF7F0 }, { 0xF90C, 0xF93A }, { 0xF93C, 0xF945 },
 { 0xF947, 0xF9FF }, { 0xFA70, 0xFAF6 }, { 0x0000, 0xFFFF },
 { 0x0000, 0xFFFD } };
#endif

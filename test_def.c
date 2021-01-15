/* Copyright (C) 2018, Project Pluto
Check assorted #defines and compiler characteristics

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA. */

#include <stdio.h>
#ifdef __BORLANDC__
#include <windows.h>
#endif

/* Predefined macros for some compilers gathered from
https://sourceforge.net/p/predef/wiki/Compilers/      */

int main( const int argc, const char **argv)
{
#if defined(__clang__)
   printf( "clang version %d.%d.%d\n", __clang_major__, __clang_minor__,
                                       __clang_patchlevel__);
#endif
#if defined(__llvm__)
   printf( "llvm : %d\n", __llvm__);
#endif
#if defined( __MINGW32__)
   printf( "MinGW32 : %d: %d.%d\n", __MINGW32__, __MINGW32_MAJOR_VERSION,
                                                 __MINGW32_MINOR_VERSION);
#endif
#if defined( __MINGW64__)
   printf( "MinGW64 : %d: %d.%d\n", __MINGW64__, __MINGW64_VERSION_MAJOR,
                                                 __MINGW64_VERSION_MINOR);
#endif
#if defined(__GNUC__)
   printf( "GCC version %d.%d.%d\n", __GNUC__, __GNUC_MINOR__,
                                     __GNUC_PATCHLEVEL__);
#endif
#if defined(_MSC_VER)
   printf( "_MSC_VER defined\n");
#endif
#if defined(__i386__)
   printf( "__i386__ defined\n");
#endif
#if defined(__WATCOMC__)
   printf( "__WATCOMC__ defined\n");
#endif
#if defined(__BYTE_ORDER__)
   printf( "__BYTE_ORDER__ defined: %u\n", __BYTE_ORDER__);
#endif
#if defined(__FLOAT_WORD_ORDER__)
   printf( "__FLOAT_WORD_ORDER__ defined: %u\n", __FLOAT_WORD_ORDER__);
#endif
#if defined(__ORDER_BIG_ENDIAN__)
   printf( "__ORDER_BIG_ENDIAN__ defined: %u\n", __ORDER_BIG_ENDIAN__);
#endif
#if defined(__ORDER_LITTLE_ENDIAN__)
   printf( "__ORDER_LITTLE_ENDIAN__ defined: %u\n", __ORDER_LITTLE_ENDIAN__);
#endif
#if defined(__ORDER_PDP_ENDIAN__)
   printf( "__ORDER_PDP_ENDIAN__ defined: %u\n", __ORDER_PDP_ENDIAN__);
#endif
#if defined(__x86_64__)
   printf( "__x86_64__ defined\n");
#endif
#if defined(__i386__)
   printf( "__i386__ defined\n");
#endif
   printf( "sizeof( int) = %d\n", (int)sizeof( int));
   printf( "sizeof( long) = %d\n", (int)sizeof( long));
#ifdef __BORLANDC__
   printf( "Borland '%x'\n", __BORLANDC__);
   printf( "sizeof( __int64) = %d\n", (int)sizeof( __int64));
   printf( "sizeof( __uint64) = %d\n", (int)sizeof( unsigned __int64));
#else
   printf( "sizeof( long long) = %d\n", (int)sizeof( long long));
#endif
   printf( "sizeof( long double) = %d\n", (int)sizeof( long double));
#ifdef __DMC__
   printf( "Digital Mars '%x'\n", __DMC__);
#endif
#ifdef __DJGPP__
   printf( "DJGPP '%x'\n", __DJGPP__);
#endif
#ifdef __TIMESTAMP__
   printf( "Timestamp = " __TIMESTAMP__ "\n");
#endif
   printf( "Date/time compiled : %s %s\n", __DATE__, __TIME__);
}

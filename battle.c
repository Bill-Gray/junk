#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

/* 'Battleship' (R)(TM) solution code.  Not complete! */

/* Convert 0<=intensity<256 value into false-color RGB.  Initially,
I displayed intensities in gray-scales.  But this is a lot easier
to see. */

static void get_pseudocolor_rgb( const int intensity, int *rgb)
{
   static const unsigned char rgbs[36] = {
                     0,   0,   0,
                     0,   0, 192,
                     0,  96, 192,
                     0, 192, 255,
                    40, 255,  96,
                    80, 255,  48,
                   168, 255,   0,
                   255, 255,   0,
                   255, 128, 128,
                   255,   0, 255,
                   255,   0, 128,
                   255,   0,   0 };
   int frac = (intensity * 11) & 0xff, i;
   const unsigned char *tptr = rgbs + 3 * (intensity * 11 / 256);

   assert( intensity >= 0 && intensity < 256);
   for( i = 0; i < 3; i++, tptr++)
      rgb[i] = (int)*tptr + frac * ((int)tptr[3] - (int)tptr[0]) / 256;
}

/* 'ship_sizes' is an array of ship sizes.  As the following main()
function shows,  it defaults to having a five-cell battleship,  four-cell
cruiser,  two three-cell ships,  and a two-cell ship.  If this code were
complete,  we'd have to make arrangements for situations where you knew
certain of those ships had been sunk.

   'Misses' and 'hits' are bit arrays indicating which cells have been
called as misses and hits.

   Based on this,  we determine a 'solution' at random,  attempting to
put down ships of the specified sizes without putting anything over a
cell that's been called as a miss,  and making sure at the end that
every 'hit' actually coincides with a ship.  If those criteria aren't
met,  we return a negative value ("no solution found").   */

int find_random_solution( int *soln, const int *ship_sizes,
               const int *misses, const int *hits)
{
   int i;

   for( i = 0; i < 10; i++)
      soln[i] = 0;
   while( *ship_sizes)
      {
      int loc = rand( ), dir = loc & 4096, loc2, j;

      loc2 = loc % 10;
      loc /= 10;
      loc %= 11 - *ship_sizes;
      for( j = *ship_sizes - 1; j >= 0; j--, loc++)
         {
         const int mask = 1 << (dir ? loc : loc2);
         const int y = (dir ? loc2 : loc);

         if( soln[y] & mask)        /* space already occupied */
            return( -1);
         soln[y] |= mask;
         if( misses[y] & mask)      /* wups!  that spot shoulda been a miss */
            return( -2);
         }
      ship_sizes++;
      }
   for( i = 0; i < 10; i++)
      if( (soln[i] & hits[i]) != hits[i])
         return( -3);
   return( 0);
}

/* Show the Battleship(R) board using VT100 (actually xterm) commands. */

void show_board( const int *soln, const int *misses, const int *hits, int prob[10][10])
{
   int i, j, maxval = 1;

   for( i = 0; i < 10; i++)
      for( j = 0; j < 10; j++)
         if( !((hits[i] >> j) & 1))
            if( maxval < prob[i][j])
               maxval = prob[i][j];
   printf( "  abcdefghij\n");
   for( i = 0; i < 10; i++)
      {
      printf( "%d|", i);
      for( j = 0; j < 10; j++)
         {
         const int mask = (1 << j);

         if( !(hits[i] & mask))
            {
            const int gray = prob[i][j] * 255 / maxval;
            int rgb[3];

            get_pseudocolor_rgb( gray, rgb);
            printf( "\033[48;2;%d;%d;%dm", rgb[2], rgb[1], rgb[0]);
            }
         if( misses[i] & mask)
            printf( "m");
         else if( hits[i] & mask)
            printf( "h");
         else if( soln[i] & mask)
            printf( "x");
         else
            printf( " ");
         if( !(hits[i] & mask))
            printf( "\033[0m");
         }
      if( i != 5)
         printf( "|%d\n", i);
      else
         {
         int j, rgb[3];

         printf( "|5   ");
         for( j = 0; j < 255; j += 5)
            {
            get_pseudocolor_rgb( j, rgb);
            printf( "\033[48;2;%d;%d;%dm ", rgb[2], rgb[1], rgb[0]);
            }
         printf( "\033[0m\n");
         }
      }
}

/* We generate a lot of random solutions fitting the current board
situation (what we know about hits,  misses,  and sinkings).  We
keep track of how often each cell would have contained a ship.  The
idea is that the cells that most frequently contain a ship are the
cells we should go after next;  they are most likely to result in hits. */

static void add_new_soln( const int *soln, int prob[10][10])
{
   int i, j;

   for( i = 0; i < 10; i++)
      for( j = 0; j < 10; j++)
          if( (soln[i] >> j) & 1)
            prob[i][j]++;
}

/* At present,  this allows you to specify hits/misses on the command
line,  with something like

./battle e4 f5 g6 h7 d3a

   meaning you got four misses,  followed by a hit at d3.  It will
proceed to generate random solutions fitting those criteria,  and once
a second,  it'll spit out a heat map showing you which cells are most
likely to have a ship in them.   */

int main( const int argc, const char **argv)
{
   int i, j, misses[10], hits[10], soln[10];
   int prob[10][10], n_boards_found = 0;
   const int ship_sizes[6] = { 5, 4, 3, 3, 2, 0};
   time_t t0 = 0;

   for( i = 0; i < 10; i++)
      misses[i] = hits[i] = 0;
   for( i = 0; i < 10; i++)
      for( j = 0; j < 10; j++)
         prob[i][j] = 0;
   for( i = 1; i < argc; i++)
      if( strlen( argv[i]) >= 2)
         {
         int x = argv[i][0] - 'a', y = argv[i][1] - '0';

         if( argv[i][2])
            hits[y] |= 1 << x;
         else
            misses[y] |= 1 << x;
         }
   while( 1)
      {
      time_t t = time( NULL);

      i = 0;
      while( find_random_solution( soln, ship_sizes, misses, hits))
         i++;
      add_new_soln( soln, prob);
      n_boards_found++;
      if( t != t0)
         {
         t0 = t;
         printf( "Soln after %d searches; %d boards\n", i, n_boards_found);
         show_board( soln, misses, hits, prob);
         }
      }
   return( 0);
}

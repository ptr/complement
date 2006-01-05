/* Time-stamp: <06/01/04 00:16:20 ptr> */

#include <stdio.h>

int main( int argc, char *argv[] )
{
  FILE *f;
  int i;

  f = fopen( "test", "w" );

  for ( i = 0; i < 1000000; ++i ) {
    fprintf( f, "%d %f\n", i, (double)i + 0.1415926 );
  }

  fclose( f );

  return 0;
}

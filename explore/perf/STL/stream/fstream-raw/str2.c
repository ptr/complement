/* Time-stamp: <06/01/04 00:21:07 ptr> */

#include <stdio.h>

int main( int argc, char *argv[] )
{
  FILE *f;
  int i;

  f = fopen( "test", "w" );

  for ( i = 0; i < 10000000; ++i ) {
    fwrite( (const void *)&i, sizeof(i), 1, f );
  }

  fclose( f );

  return 0;
}

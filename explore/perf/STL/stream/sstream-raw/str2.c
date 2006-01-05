/* Time-stamp: <06/01/04 01:31:49 ptr> */

#include <stdio.h>

int main( int argc, char *argv[] )
{
  char *buf;
  size_t sz = 16;
  int i;

  buf = malloc( sz );

  for ( i = 0; i < 10000000; ++i ) {
    if ( sz < (i * sizeof(i) + sizeof(i)) ) {
      char *tmp;

      tmp = malloc( sz * 2 );
      memcpy( tmp, buf, sz );
      free( buf );
      buf = tmp;
      sz = sz * 2;
    }
    memcpy( buf + i * sizeof(i), &i, sizeof(i) );
  }

  free( buf );

  return 0;
}

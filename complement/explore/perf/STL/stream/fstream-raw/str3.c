/* Time-stamp: <06/01/04 00:21:07 ptr> */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{
  int f;
  int i;

  f = open( "test", O_WRONLY | O_CREAT | O_TRUNC, 0666 );

  for ( i = 0; i < 10000000; ++i ) {
    write( f, (const void *)&i, sizeof(i) );
  }

  close( f );

  return 0;
}

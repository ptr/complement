#include <stdlib.h>
#include <values.h>

int main( int argc, char **argv )
{
  char buf[128];
  int pt, sign;

  fcvt_r( 1.0e+22, 0, &pt, &sign, buf, 128 );

  printf( "%s, %d\n", buf, DBL_MAX_10_EXP );

  printf( "%f\n", 1.0e+23 );

  return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <ieee754.h>

union D {
  unsigned char c[16];
  double d;
};

union LD {
  unsigned char c[16];
  long double d;
};

union U {
  unsigned u[2];
  double d;
};

int main( int argc, char **argv )
{
  union D d;
  union ieee754_double ieee754;
  union U u;
  union LD ld;
  int i;

  for ( i = 0; i < 16; ++i ) {
    d.c[i] = 0;
  }

  d.d = 1.0;
  ieee754.d = 1.0;
  u.u[0] = 0;
  u.u[1] = 0x3ff << 20;

  for ( i = 0; i < 16; ++i ) {
    printf( "%.2x ", d.c[i] );
  }
  printf( "\n" );

  for ( i = 0; i < 8; ++i ) {
    printf( "%.2x ", *((unsigned char *)&ieee754.d + i) );
  }
  printf( "\n" );

  printf( "%.1x %.3x %.5x %.8x\n", ieee754.ieee.negative, ieee754.ieee.exponent, ieee754.ieee.mantissa0, ieee754.ieee.mantissa1 );

  for ( i = 0; i < 8; ++i ) {
    printf( "%.2x ", *((unsigned char *)&u.d + i) );
  }
  printf( "\n" );

  printf( "%d\n", sizeof(unsigned short) );
  printf( "%d\n", sizeof(long double) );

  ld.d = 1.0;

  for ( i = 0; i < 16; ++i ) {
    printf( "%.2x ", ld.c[i] );
  }
  printf( "\n" );

  return 0;
}

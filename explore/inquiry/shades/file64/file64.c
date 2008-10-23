#define _FILE_OFFSET_BITS 64

#include <stdio.h>

int main()
{
  FILE *f = fopen( "sample", "r" );
  fclose( f );
  return 0;
}

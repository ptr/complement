#include <stdio.h>

int main()
{
  printf( "glibc version %d.%d\n", __GLIBC__, __GLIBC_MINOR__ );
  return 0;
}

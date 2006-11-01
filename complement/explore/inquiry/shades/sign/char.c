#include <stdio.h>

int main()
{
  char c1 = 0xff;
  char c2 = 0xff;

  unsigned int ui1 = (unsigned)c1;
  unsigned ui2 = (unsigned)c2;

  printf( "%x\n", ui1 );

  return 0;
}

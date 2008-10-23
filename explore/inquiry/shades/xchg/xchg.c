#include <stdio.h>
#include <asm/system.h>
#include <asm/atomic.h>

int main()
{
  int x = 1;
  int y = 2;
  int z = 3;

  atomic_t a = ATOMIC_INIT(5);

  printf( "%d %d %d\n", x, y, z );

  z = xchg( &x, y );

  printf( "%d %d %d\n", x, y, z );

  printf( "%d\n", atomic_read( &a ) );

  atomic_inc( &a );

  printf( "%d\n", atomic_read( &a ) );

  atomic_dec( &a );

  printf( "%d\n", atomic_read( &a ) );

  return 0;
}

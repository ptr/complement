#include <stdio.h>
#include <stdint.h>

int main()
{
  char c1 = 0xff;
  char c2 = 0xff;

  unsigned int ui1 = (unsigned)c1;
  unsigned ui2 = (unsigned)c2;
  /* unsigned ui3 = ~((1 << (sizeof(unsigned) * 8 - 1)) >> 3); */
  unsigned ui3 = ~((1U << 31U) >> 3U);
  uint64_t ui4 = ~((1ULL << 63ULL) >> 3ULL);
  union {
   uint64_t i64;
   struct {
     uint32_t lo;
     uint32_t hi;
   } i32;
  } ui5;

  ui5.i64 = ui4;

  printf( "%x %x %llx %x\n", ui1, ui3, ui4, ui5.i32.hi );

  ui4 = ((~0ULL) / 2ULL) >> 52ULL;

  printf( "%llx\n", ui4 );
  ui4 = ~0ULL >> 53;
  printf( "%llx\n", ui4 );
  ui4 = ~0ULL & (~0ULL << 53);
  printf( "%llx\n", ui4 );


  return 0;
}

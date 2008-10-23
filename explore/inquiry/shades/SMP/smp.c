#include <stdio.h>
#include <pthread.h>
#include <asm/atomic.h>

#ifdef CONFIG_SMP
#error CONFIG_SMP present!
#else
#error CONFIG_SMP absent!
#endif

int main()
{
  return 0;
}

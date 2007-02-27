#include "my.h"

int other()
{
  A<int> a;
  a.use_it() += 1;

  return a.use_it();
}


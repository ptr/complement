#include <iostream> 
// #include "my.h"

extern int other();
 
int main(void) 
{
  // A<int> a;
  // a.use_it() += 1;
  // int v1 = a.use_it();
  // int v2 = other();
  std::cout << "Hello, world! " << /* v1 << " " << v2 << */ std::endl; 
  return 0; 
} 

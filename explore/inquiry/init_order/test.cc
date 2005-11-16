/*
 * This is a test for initialization call order of data members,
 * with respect to explicit initialization sequence.
 * In accordance to Standard, call order should be same as declaration
 * order, ISO IEC 14882 1998, 12.6.2 par 5 (third item), i.e. this
 * program has to type:
 *   B::b <- 2
 *   A::a <- 1
 * but not
 *   A::a <- 1
 *   B::b <- 2
 */

#include <iostream>

using namespace std;

class A
{
  public:
    A() :
     a(1)
     { cerr << "A::a <- " << a << endl; }

    int a;
};

class B
{
  public:
    B() :
      b(2)
    { cerr << "B::b <- " << b << endl; }

    int b;
};

class C
{
  public:
    C() :
      a(), // <<<<----- order is reversed relative declaration order
      b()
    { }

   B b;    // <<<<-----
   A a;
};

int main()
{
  C c;

  return 0;
}

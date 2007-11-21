#include <string>
#include <iostream>

using namespace std;

class A
{
  public:
    bool a();
    bool b();
    bool c();
};

bool A::a()
{
  cerr << "A::a" << endl;
  return true;
}

bool A::b()
{
  cerr << "A::b" << endl;
  return true;
}

bool A::c()
{
  cerr << "A::c" << endl;
  return true;
}

int main()
{
  cerr << &A::a << " " << &A::b << " " << &A::c << endl;

  A x;

  bool (A::*mem)() = &A::b;

  (x.*mem)();
  // cerr << &x.a << " " << &x.b << " " << &x.c << endl;

  return 0;
}

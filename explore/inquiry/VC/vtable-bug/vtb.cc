#include <iostream>
#include <mt/xmt.h>

using namespace std;
using namespace __impl;

class Base
{
  public:
    Base()
      { }

    virtual ~Base()
      { }

    virtual void func() = 0;
};

class Derive :
    public Base
{
  public:
    Derive()
      { }

    virtual ~Derive()
      { }

    virtual void func()
      { cout << "Derive::func" << endl; }
};


int thr( void *p )
{
  Derive *d = reinterpret_cast<Derive *>(p);

  d->func();

  return 0;
}


int main()
{
  Derive d;

  Thread t;

  t.launch( thr, &d );

  t.join();

  return 0;
}

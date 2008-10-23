#include <iostream>

using namespace std;

template <class T>
class A
{
  public:
    void f()
      { T *v = new T(); v->f(); }
};

template <class T, void (T::*M)() = &T::f >
class C
{
  public:
    void f()
      { T *v = new T(); (v->*M)(); }
};

class B
{
  public:
    typedef void (B::*mf)();

    void f()
      { cerr << "B::f\n"; }

    void g()
      { cerr << "B::g\n"; }
};

int main()
{
  A<B> a;

  a.f();

  C<B,&B::f> c;

  c.f();

  C<B> c1;

  c1.f();

  C<B,&B::g> c2;

  c2.f();

  return 0;
}


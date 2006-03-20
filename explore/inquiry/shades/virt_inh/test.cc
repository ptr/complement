#include <iostream>

using namespace std;

class X
{
  public:
    void f() { cerr << "A::f" << endl; }
};

class A
{
  public:
    virtual void f() { cerr << "A::f" << endl; }
};

class B : public A
{
  public:
    virtual void f() { cerr << "B::f" << endl; }
};

class C : public A
{
  public:
    virtual void f() { cerr << "C::f" << endl; }
};

class D : public B, public C
{
  public:
    virtual void f() { cerr << "D::f" << endl; }
};

class Bv : virtual public A
{
  public:
    virtual void f() { cerr << "Bv::f" << endl; }
};

class Cv : virtual public A
{
  public:
    virtual void f() { cerr << "Cv::f" << endl; }
};

class Dv : public Bv, public Cv
{
  public:
    virtual void f() { cerr << "Dv::f" << endl; }
};


int main()
{
  // Dv d;

  // d.f();

  cerr << sizeof(&X::f) << '\n'
       << sizeof(&A::f) << '\n'
       << sizeof(&B::f) << '\n'
       << sizeof(&C::f) << '\n'
       << sizeof(&D::f) << '\n'
       << sizeof(&Bv::f) << '\n'
       << sizeof(&Cv::f) << '\n'
       << sizeof(&Dv::f) << '\n'
       << endl;

  return 0;
}

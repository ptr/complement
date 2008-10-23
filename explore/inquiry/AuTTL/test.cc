#include <iostream>

using namespace std;

class Obj
{
  public:

   Obj( int _v ) :
     v( _v )
   { cerr << "Obj(" << v << ")" << endl; }

   Obj& operator=( const Obj& o )
   { cerr << "=(" << o.v << ")" << endl; /* v = o.v; */ }

   Obj( const Obj& o )
   { v = o.v; cerr << "O(Obj" << o.v << ")" << endl; }

   ~Obj()
   { cerr << "~Obj(" << v << ")" << endl; }

   int v;
};

Obj f()
{
  Obj v(1);
  const Obj& v2 = v;

  return v2;
}

const Obj& g() // ill-formed!
{
  Obj v(2);
  const Obj& v3 = v; // <---

  return v3; // <---
}

Obj h()
{
  Obj v(4);
  Obj v2(5);

  return v2;
}

int main()
{
  {
    Obj v0( 0 );

    v0 = f();
  }
  {
    Obj v1( 3 );

    v1 = g();
  }
  {
    Obj v2( 6 );

    v2 = h();
  }

  return 0;
}


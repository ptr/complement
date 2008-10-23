#include <iostream>

using namespace std;

class Obj
{
  public:

   Obj( int _v ) :
     v( _v )
   { cerr << "Obj(" << v << ")" << endl; }

   Obj& operator=( const Obj& o )
   { v = o.v; cerr << "=(" << o.v << ")" << endl; }

   Obj( const Obj& o )
   { v = o.v; cerr << "O(Obj" << o.v << ")" << endl; }

   ~Obj()
   { cerr << "~Obj(" << v << ")" << endl; }

   int v;
   static int q;
};

int Obj::q;

Obj f()
{
  Obj v1(1);
  Obj v2(2);

  return v2;
}

int main()
{
  Obj v0( 0 );

  v0 = f();

  return 0;
}


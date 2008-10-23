#include <stdio.h>

struct A
{
  A() { printf( "%p %p\n", &buf[0], this ); }
  A( int j ) : i(j) { printf( "%p %p\n", &buf[0], this ); }
  A( const A& a ) : i(a.i) { printf( "%p %p\n", &buf[0], this ); }
  A& operator =( const A& a ) { printf( "%p %p\n", &buf[0], this ); return *this; }
  A& operator =( int j ) { printf( "%p %p\n", &buf[0], this ); return *this; }

  ~A() { printf( "%p %p\n", &buf[0], this ); }

  int i;
  char buf[2];
};

struct B
{
  A a1;
};

const A a1 = 1;

int main()
{
  printf( "-- 0\n" );
  {
    const B b[] = { {a1} };
  }
  printf( "-- 1\n" );
  {
    const B b;
  }
  printf( "-- 2\n" );

  return 0;
}

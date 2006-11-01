#include <stdio.h>
#include <string>
// #include <iostream>
// #include <iomanip>

using namespace std;

#if 1 

struct A
{
  A() { /* cerr << "A()\n"; */ }
  A( int j ) : i(j) { /* cerr << "A( int )\n"; */ }
  A( const A& a ) : i(a.i) { /* cerr << "A( const A& )\n"; */ }
  A& operator =( const A& a ) { /* i = a.i; cerr << "operator =( const A& )\n"; */ return *this; }
  A& operator =( int j ) { /* i = j; cerr << "operator =( int )\n"; return *this; */ }

  ~A() { /* cerr << "~A()\n"; */ }

  int i;
};

struct B
{
  A a1;
  // A a2;
};

const A a1 = 1;
// const A a2 = 2;

// const B b[] = { {a1,a2} /* , {a1,a2} */ };
#endif

const string CON_ONE = "string_1";
// const string CON_TWO = "string_2";

struct StrOne
{
  string partOne;
    // string partTwo;
};

// #define NUM_PARAMS 2

// const StrOne general[NUM_PARAMS] = {
//	{string(CON_ONE), string(CON_TWO)},
//	{string(CON_ONE), string(CON_TWO)}      
//};

// const StrOne general[] = { {CON_ONE} };

// const string str[] = { CON_ONE };

//const string s[] = { CON_ONE, CON_TWO };

int main()
{
  // const StrOne general[] = { {CON_ONE} };
  const B b[] = { {a1} };
  // const B b;
  printf( "Q\n" );
  // string temp = general[0].partOne;
  // A i = b[0].a1;
  return 0;
}

// -*- C++ -*- Time-stamp: <02/09/25 12:11:17 ptr>

#include <iostream>

using namespace std;

class A
{
  public:
    A()
     { cerr << "Hello from static!" << endl; }
};

A a;

int main( int argc, char * const *argv )
{
  cerr << "Hello, world!" << endl;

  return 0;
}

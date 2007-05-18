#include <iostream>
#include <mt/xmt.h>

#include "vtime.h"

using namespace std;
using namespace xmt;
using namespace vt;

int main()
{
  Condition cnd;

  cnd.set(false);

  cerr << "Hello, world!" << endl;

  Proc m1( 100 );
  Proc m2( 101 );
  Proc r1( 102 );
  Proc r3( 103 );

  stem::Event_base<VTmess> mess( MESS );
  
  mess.dest( 101 );
  mess.value().mess = "Hello!";

  m1.Send( mess );

  cnd.wait();

  return 0;
}


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

  vtime_group_type gvt;

  gvt.first = 0; // group
  gvt.second += make_pair( 101, 1 );

  mess.value().mess = "Hello!";
  mess.value().grp = 0;
  mess.value().gvt += gvt;

  m1.Send( mess );

  cnd.wait();

  return 0;
}


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
  Proc r2( 103 );

  m1.add_group( 0 );
  m2.add_group( 0 );
  r1.add_group( 0 );

  m1.add_group( 1 );
  m2.add_group( 1 );
  r2.add_group( 1 );

  stem::Event_base<VTmess> mess( MESS );
  
  mess.dest( m2.self_id() );

  mess.value().mess = "Hello!";
  mess.value().grp = 0;
  mess.value().gvt.gvt[0][m1.self_id()] = 1;

  m1.Send( mess );

  mess.dest( r1.self_id() );

  m1.Send( mess );

  // mess.value().gvt.gvt[0][m1.self_id()] += 1;

  // mess.value().mess = "How are you?";
  // m1.Send( mess );

  cnd.wait();

  return 0;
}


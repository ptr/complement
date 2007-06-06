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

  m1.add_group_member( 0, m2.self_id() );
  m1.add_group_member( 0, r1.self_id() );

  r1.add_group_member( 0, m1.self_id() );
  r1.add_group_member( 0, m2.self_id() );

  m2.add_group_member( 0, m1.self_id() );
  m2.add_group_member( 0, r1.self_id() );

  m1.add_group( 1 );
  m2.add_group( 1 );
  r2.add_group( 1 );

  m1.add_group_member( 1, m2.self_id() );
  m1.add_group_member( 1, r2.self_id() );

  r2.add_group_member( 1, m1.self_id() );
  r2.add_group_member( 1, m2.self_id() );

  m2.add_group_member( 1, m1.self_id() );
  m2.add_group_member( 1, r2.self_id() );

  r1.SendVC( 0, "Hello!" );

  m1.SendVC( 0, "How are you?" );

  cnd.wait();

  return 0;
}

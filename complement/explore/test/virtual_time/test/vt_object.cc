// -*- C++ -*- Time-stamp: <07/07/25 22:09:32 ptr>

#include "vt_operations.h"

// #include <boost/lexical_cast.hpp>

#include <iostream>
#include <vtime.h>

using namespace vt;
using namespace std;

int EXAM_IMPL(vtime_operations::vt_object)
{
  detail::vtime_obj_rec ob;

  const group_type gr0 = 0;
  const group_type gr1 = 1;
  const group_type gr2 = 2;
  oid_type obj0; obj0.addr = 0;
  oid_type obj1; obj1.addr = 1;
  oid_type obj2; obj2.addr = 2;

  ob.add_group( gr0 );
  // ob.add_group_member( gr0, obj0 );
  // ob.add_group_member( gr0, obj1 );
  // ob.add_group_member( gr0, obj2 );

  // gvtime gvt;
  // gvt[gr0][obj1] = 1;

  VTmess mess;
  VTmess mess_bad;

  mess_bad.code = mess.code = 1;
  mess_bad.src =  mess.src = obj1;
  mess_bad.gvt[gr0][obj1] = mess.gvt[gr0][obj1] = 1;
  mess_bad.grp = mess.grp = gr0;
  mess_bad.mess = mess.mess = "data";

  // cerr << ob.vt[gr0] << endl;
  // cerr << "===========\n";

  vtime chk;

  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );

  EXAM_REQUIRE( ob.deliver(mess) ); // ack

  chk[obj1] += 1;

  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );

  // cerr << ob.vt[gr0] << endl;
  // cerr << "===========\n";

  ++mess.gvt[gr0][obj1];

  EXAM_REQUIRE( ob.deliver(mess) ); // ack

  chk[obj1] += 1;

  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );

  ++mess.gvt[gr0][obj1];

  try {
    EXAM_CHECK( !ob.deliver(mess_bad) ); // nac: too old (out of order)
    EXAM_ERROR( "exception expected" );
  }
  catch ( const out_of_range& ) {
    EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );
  }

  mess_bad.gvt[gr0][obj1] = mess.gvt[gr0][obj1] + 1;

  EXAM_REQUIRE( !ob.deliver(mess_bad) ); // nac: too new (out of order)

  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );

  EXAM_REQUIRE( ob.deliver(mess) ); // ack

  chk[obj1] += 1;
  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );

  mess_bad.gvt[gr0][obj1] = ++mess.gvt[gr0][obj1];

  // cerr << ob.vt[gr0] << endl;
  // cerr << "===========\n";

  // ----

  VTmess mess2;

  mess2.code = 1;
  mess2.src = obj2;
  mess2.gvt[gr0][obj2] = 1;
  mess2.grp = gr0;
  mess2.mess = "data";

  mess_bad.gvt[gr0][obj2] = 1;

  EXAM_CHECK( !ob.deliver(mess_bad) ); // nac: obj0 don't seen mess from obj2, but obj1 seen mess from obj2

  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );

  EXAM_REQUIRE( ob.deliver(mess2) ); // ack: obj0 see first mess from obj2

  chk[obj2] += 1;
  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );

  ++mess2.gvt[gr0][obj2];

  EXAM_REQUIRE( ob.deliver(mess_bad) ); // ack: now obj0 and obj1 sync dependency from obj2

  // cerr << ob.vt[gr0] << endl;
  // cerr << "===========\n";

  chk[obj1] += 1;
  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );

  mess_bad.gvt[gr0][obj1] = ++mess.gvt[gr0][obj1];
  mess.gvt[gr0][obj2] = 1;
  mess_bad.gvt[gr0][obj2] = 0;

  // ----

  ob.add_group( gr1 );
  // ob.add_group_member( gr1, obj0 );
  // ob.add_group_member( gr1, obj1 );
  // ob.add_group_member( gr1, obj2 );

  mess_bad.grp = gr2;

  vtime chk1;

  try {
    EXAM_CHECK( ob.deliver(mess_bad) ); // nac: ob not member of group gr2
    EXAM_ERROR( "exception expected" );
  }
  catch ( const domain_error& ) {
    EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );
    EXAM_REQUIRE( (chk1 <= ob[gr1]) && (chk1 >= ob[gr1]) );
    EXAM_REQUIRE( (chk1 <= ob[gr2]) && (chk1 >= ob[gr2]) );
  }

  // ----

  mess_bad.grp = gr0;
  mess_bad.gvt[gr1][obj2] = 1;

  EXAM_REQUIRE( !ob.deliver(mess_bad) ); // nac: obj1 recieve new event in group gr1 from obj2

  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );
  EXAM_REQUIRE( (chk1 <= ob[gr1]) && (chk1 >= ob[gr1]) );

  // cerr << "===========\n";
  // cerr << ob.vt[gr0] << endl;
  // cerr << "===========\n";

  VTmess mess3;

  mess3.code = 1;
  mess3.src = obj2;
  mess3.gvt[gr1][obj2] = 1;
  mess3.grp = gr1;
  mess3.mess = "data";

  EXAM_REQUIRE( ob.deliver(mess3) ); // ack: see event from obj2 in group gr1

  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );
  chk1[obj2] += 1;
  EXAM_REQUIRE( (chk1 <= ob[gr1]) && (chk1 >= ob[gr1]) );

  ++mess3.gvt[gr1][obj2];

  // cerr << "===========\n";
  // cerr << /* ob.vt[gr0] */ mess_bad.gvt[gr0] << endl;
  // cerr << /* ob.vt[gr0] */ mess_bad.gvt[gr1] << endl;
  // cerr << "===========\n";

  // cerr << "2 ==========\n";
  EXAM_REQUIRE( ob.deliver(mess_bad) ); // ack: now we know about event in group gr1 

  chk[obj1] += 1;
  EXAM_REQUIRE( (chk <= ob[gr0]) && (chk >= ob[gr0]) );
  EXAM_REQUIRE( (chk1 <= ob[gr1]) && (chk1 >= ob[gr1]) );

  return EXAM_RESULT;
}

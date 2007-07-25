// -*- C++ -*- Time-stamp: <07/07/23 23:46:02 ptr>

#include <exam/suite.h>

#include <boost/lexical_cast.hpp>

#include <string>
#include <iostream>

#include <vtime.h>

using namespace vt;
using namespace std;

struct vtime_operations
{
  int EXAM_DECL(vt_compare);
  int EXAM_DECL(vt_add);
  int EXAM_DECL(vt_diff);
  int EXAM_DECL(vt_max);

  int EXAM_DECL(gvt_add);

  int EXAM_DECL(VTMess_core);

  int EXAM_DECL(vt_object);

  int EXAM_DECL(VTDispatch);
};

int EXAM_IMPL(vtime_operations::vt_compare)
{
  vtime_type vt1;
  vtime_type vt2;

  vt1[1] = 1;
  vt1[2] = 1;

  vt2[1] = 1;
  vt2[2] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( vt2 <= vt1 );
  EXAM_CHECK( vt1 >= vt2 );
  EXAM_CHECK( vt2 >= vt1 );

  vt2[3] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( !(vt2 <= vt1) );
  EXAM_CHECK( vt2 >= vt1 );

  vt1.clear();
  vt2.clear();

  vt1[1] = 1;

  vt2[1] = 1;
  vt2[3] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( !(vt2 <= vt1) );
  
  vt1[2] = 1;

  EXAM_CHECK( !(vt1 <= vt2) );
  EXAM_CHECK( !(vt2 <= vt1) );
}

int EXAM_IMPL(vtime_operations::vt_add)
{
  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1[1] = 1;
  vt1[2] = 1;

  vt3 = vt1 + vt2;

  EXAM_CHECK( vt1 <= vt3 );
  EXAM_CHECK( vt3 <= vt1 );

  vt2[2] = 1;

  vt3 = vt1 + vt2;

  vt4[1] = 1;
  vt4[2] = 2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt4.clear();

  vt2[3] = 1;

  vt3 = vt1 + vt2;

  vt4[1] = 1;
  vt4[2] = 2;
  vt4[3] = 1;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::vt_diff)
{
  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1[1] = 1;
  vt1[2] = 1;

  vt3 = vt1 - vt2;

  EXAM_CHECK( vt1 <= vt3 );
  EXAM_CHECK( vt3 <= vt1 );

  vt2[1] = 1;

  vt3 = vt1 - vt2;

  vt4[2] = 1;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2[2] = 1;

  vt4.clear();

  vt3 = vt1 - vt2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2.clear();

  vt2[3] = 1;
  
  try {
    vt3 = vt1 - vt2;
    EXAM_ERROR( "Virtual Times are incomparable" );
  }
  catch ( const std::range_error& err ) {
    EXAM_CHECK( true );
  }

  vt2.clear();

  vt2[2] = 2;

  try {
    vt3 = vt1 - vt2;
    EXAM_ERROR( "Virtual Times are incomparable" );
  }
  catch ( const std::range_error& err ) {
    EXAM_CHECK( true );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::vt_max)
{
  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1[1] = 1;
  vt1[2] = 1;

  vt3 = vt::max( vt1, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[1] = 1;
  
  vt3 = vt::max( vt1, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[2] = 1;

  vt3 = vt::max( vt1, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[3] = 1;

  vt3 = vt::max( vt1, vt2 );

  vt4[1] = 1;
  vt4[2] = 1;
  vt4[3] = 1;
 
  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2.clear();

  vt2[1] = 1;
  vt2[2] = 2;

  vt4.clear();

  vt3 = vt::max( vt1, vt2 );

  vt4[1] = 1;
  vt4[2] = 2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2[3] = 4;

  vt3 = vt::max( vt1, vt2 );

  vt4[3] = 4;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::gvt_add)
{
  {
    gvtime_type gvt1;
    gvtime_type gvt2;

    vtime_type vt1;
    vtime_type vt2;

    vt1[1] = 1;
    vt1[2] = 1;

    vt2[1] = 1;
    vt2[2] = 1;

    gvt1[0] = vt1;
    gvt2[0] = vt2;

    gvt1 += gvt2;

    EXAM_CHECK( gvt1[0][1] == 2 );
    EXAM_CHECK( gvt1[0][2] == 2 );
    EXAM_CHECK( gvt1[0][0] == 0 );
    EXAM_CHECK( gvt1[1][1] == 0 );
    EXAM_CHECK( gvt1[1][2] == 0 );
  }
  {
    gvtime_type gvt1;
    gvtime_type gvt2;

    vtime_type vt1;
    vtime_type vt2;

    vt1[1] = 1;
    vt1[2] = 1;

    vt2[1] = 1;
    vt2[2] = 1;

    gvt1[0] = vt1;
    gvt2[1] = vt2;

    gvt1 += gvt2;

    EXAM_CHECK( gvt1[0][1] == 1 );
    EXAM_CHECK( gvt1[0][2] == 1 );
    EXAM_CHECK( gvt1[0][0] == 0 );
    EXAM_CHECK( gvt1[1][1] == 1 );
    EXAM_CHECK( gvt1[1][2] == 1 );
  }
  {
    gvtime_type gvt1;

    vtime_type vt1;
    vtime_type vt2;

    vt1[1] = 1;
    vt1[2] = 1;

    vt2[1] = 1;
    vt2[2] = 1;

    gvt1[0] = vt1;

    gvt1 += make_pair( 1, vt2 );

    EXAM_CHECK( gvt1[0][1] == 1 );
    EXAM_CHECK( gvt1[0][2] == 1 );
    EXAM_CHECK( gvt1[0][0] == 0 );
    EXAM_CHECK( gvt1[1][1] == 1 );
    EXAM_CHECK( gvt1[1][2] == 1 );
  }

  return EXAM_RESULT;
}

class VTM_handler :
    public stem::EventHandler
{
  public:
    VTM_handler();
    VTM_handler( stem::addr_type id );
    VTM_handler( stem::addr_type id, const char *info );
    ~VTM_handler();

    void handlerE( const stem::Event_base<VTmess>& );
    void handlerV( const VTmess& );

    void wait();

    stem::code_type code;
    oid_type src;    
    gvtime gvt;
    group_type grp;
    std::string mess;

  private:
    xmt::condition cnd;

    DECLARE_RESPONSE_TABLE( VTM_handler, stem::EventHandler );
};

#define VT_MESS  0x1201

VTM_handler::VTM_handler() :
    EventHandler()
{
  cnd.set( false );
}

VTM_handler::VTM_handler( stem::addr_type id ) :
    EventHandler( id )
{
  cnd.set( false );
}

VTM_handler::VTM_handler( stem::addr_type id, const char *info ) :
    EventHandler( id, info )
{
  cnd.set( false );
}

VTM_handler::~VTM_handler()
{
  // cnd.wait();
}

void VTM_handler::handlerE( const stem::Event_base<VTmess>& ev )
{
  code = ev.value().code;
  src = ev.value().src;
  gvt = ev.value().gvt;
  grp = ev.value().grp;
  mess = ev.value().mess;

  PushState( 1 );
  cnd.set( true );
}

void VTM_handler::handlerV( const VTmess& m )
{
  code = m.code;
  src = m.src;
  gvt = m.gvt;
  grp = m.grp;
  mess = m.mess;

  PopState();
  cnd.set( true );
}

void VTM_handler::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

DEFINE_RESPONSE_TABLE( VTM_handler )
  EV_Event_base_T_( ST_NULL, VT_MESS, handlerE, VTmess )
  EV_T_( 1, VT_MESS, handlerV, VTmess )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VTMess_core)
{
  VTM_handler h;

  stem::Event_base<VTmess> ev( VT_MESS );

  ev.dest( h.self_id() );
  ev.value().code = 2;
  ev.value().src = 3;
  ev.value().gvt[0][0] = 1;
  ev.value().gvt[0][1] = 2;
  ev.value().gvt[1][0] = 3;
  ev.value().gvt[1][1] = 4;
  ev.value().grp = 7;
  ev.value().mess = "data";

  h.Send( ev );

  h.wait();

  EXAM_CHECK( h.code == 2 );
  EXAM_CHECK( h.src == 3 );
  EXAM_CHECK( h.gvt[0][0] == 1 );
  EXAM_CHECK( h.gvt[0][1] == 2 );
  EXAM_CHECK( h.gvt[1][0] == 3 );
  EXAM_CHECK( h.gvt[1][1] == 4 );
  EXAM_CHECK( h.grp == 7 );
  EXAM_CHECK( h.mess == "data" );

  ev.value().code = 3;
  ev.value().mess = "more data";

  h.Send( ev );

  h.wait();

  EXAM_CHECK( h.code == 3 );
  EXAM_CHECK( h.src == 3 );
  EXAM_CHECK( h.gvt[0][0] == 1 );
  EXAM_CHECK( h.gvt[0][1] == 2 );
  EXAM_CHECK( h.gvt[1][0] == 3 );
  EXAM_CHECK( h.gvt[1][1] == 4 );
  EXAM_CHECK( h.grp == 7 );
  EXAM_CHECK( h.mess == "more data" );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::vt_object)
{
  vtime_obj_rec ob;

  const group_type gr0 = 0;
  const group_type gr1 = 1;
  const group_type gr2 = 2;
  const oid_type obj0 = 0;
  const oid_type obj1 = 1;
  const oid_type obj2 = 2;

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

  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );

  EXAM_REQUIRE( ob.deliver(mess) ); // ack

  chk[obj1] += 1;

  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );

  // cerr << ob.vt[gr0] << endl;
  // cerr << "===========\n";

  ++mess.gvt[gr0][obj1];

  EXAM_REQUIRE( ob.deliver(mess) ); // ack

  chk[obj1] += 1;

  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );

  ++mess.gvt[gr0][obj1];

  try {
    EXAM_CHECK( !ob.deliver(mess_bad) ); // nac: too old (out of order)
    EXAM_ERROR( "exception expected" );
  }
  catch ( const out_of_range& ) {
    EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );
  }

  mess_bad.gvt[gr0][obj1] = mess.gvt[gr0][obj1] + 1;

  EXAM_REQUIRE( !ob.deliver(mess_bad) ); // nac: too new (out of order)

  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );

  EXAM_REQUIRE( ob.deliver(mess) ); // ack

  chk[obj1] += 1;
  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );

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

  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );

  EXAM_REQUIRE( ob.deliver(mess2) ); // ack: obj0 see first mess from obj2

  chk[obj2] += 1;
  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );

  ++mess2.gvt[gr0][obj2];

  EXAM_REQUIRE( ob.deliver(mess_bad) ); // ack: now obj0 and obj1 sync dependency from obj2

  // cerr << ob.vt[gr0] << endl;
  // cerr << "===========\n";

  chk[obj1] += 1;
  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );

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
    EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );
    EXAM_REQUIRE( (chk1 <= ob.vt[gr1]) && (chk1 >= ob.vt[gr1]) );
    EXAM_REQUIRE( (chk1 <= ob.vt[gr2]) && (chk1 >= ob.vt[gr2]) );
  }

  // ----

  mess_bad.grp = gr0;
  mess_bad.gvt[gr1][obj2] = 1;

  EXAM_REQUIRE( !ob.deliver(mess_bad) ); // nac: obj1 recieve new event in group gr1 from obj2

  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );
  EXAM_REQUIRE( (chk1 <= ob.vt[gr1]) && (chk1 >= ob.vt[gr1]) );

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

  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );
  chk1[obj2] += 1;
  EXAM_REQUIRE( (chk1 <= ob.vt[gr1]) && (chk1 >= ob.vt[gr1]) );

  ++mess3.gvt[gr1][obj2];

  // cerr << "===========\n";
  // cerr << /* ob.vt[gr0] */ mess_bad.gvt[gr0] << endl;
  // cerr << /* ob.vt[gr0] */ mess_bad.gvt[gr1] << endl;
  // cerr << "===========\n";

  // cerr << "2 ==========\n";
  EXAM_REQUIRE( ob.deliver(mess_bad) ); // ack: now we know about event in group gr1 

  chk[obj1] += 1;
  EXAM_REQUIRE( (chk <= ob.vt[gr0]) && (chk >= ob.vt[gr0]) );
  EXAM_REQUIRE( (chk1 <= ob.vt[gr1]) && (chk1 >= ob.vt[gr1]) );

  return EXAM_RESULT;
}


class Dummy :
    public stem::EventHandler
{
  public:
    Dummy();
    Dummy( stem::addr_type id );
    Dummy( stem::addr_type id, const char *info );
    ~Dummy();

    void handler( const stem::Event& );

    void wait();
    std::string msg;

  private:
    xmt::condition cnd;

    DECLARE_RESPONSE_TABLE( Dummy, stem::EventHandler );
};

#define VT_MESS2  0x1202

Dummy::Dummy() :
    EventHandler()
{
  cnd.set( false );
}

Dummy::Dummy( stem::addr_type id ) :
    EventHandler( id )
{
  cnd.set( false );
}

Dummy::Dummy( stem::addr_type id, const char *info ) :
    EventHandler( id, info )
{
  cnd.set( false );
}

Dummy::~Dummy()
{
  // cnd.wait();
}

void Dummy::handler( const stem::Event& ev )
{
  msg = ev.value();

  cnd.set( true );
}

void Dummy::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

DEFINE_RESPONSE_TABLE( Dummy )
  EV_EDS( ST_NULL, VT_MESS2, handler )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VTDispatch)
{
  vt::VTDispatcher dsp;
  Dummy dummy1;
  Dummy dummy2;

  dsp.Subscribe( dummy1.self_id(), 1, 0 );
  dsp.Subscribe( dummy2.self_id(), 2, 0 );

  stem::Event ev( VT_MESS2 );
  ev.src( dummy1.self_id() );

  ev.value() = "hello";

  dsp.VTSend( ev, 0 );

  dummy2.wait();

  EXAM_CHECK( dummy2.msg == "hello" );
  EXAM_CHECK( dummy1.msg == "" );

  return EXAM_RESULT;
}


int EXAM_DECL(vtime_test_suite);

int EXAM_IMPL(vtime_test_suite)
{
  exam::test_suite::test_case_type tc[3];

  exam::test_suite t( "virtual time operations" );

  vtime_operations vt_oper;

  t.add( &vtime_operations::vt_max, vt_oper, "Max",
         tc[1] = t.add( &vtime_operations::vt_add, vt_oper, "Additions",
                        tc[0] = t.add( &vtime_operations::vt_compare, vt_oper, "Compare" ) ) );
  t.add( &vtime_operations::vt_diff, vt_oper, "Differences", tc[0] );

  t.add( &vtime_operations::VTMess_core, vt_oper, "VTmess core transfer", 
         tc[2] = t.add( &vtime_operations::gvt_add, vt_oper, "Group VT add", tc[1] ) );

  t.add( &vtime_operations::vt_object, vt_oper, "VT order", tc[2] );

  return t.girdle();
}

int main( int, char ** )
{

  return vtime_test_suite(0);
}

// -*- C++ -*- Time-stamp: <07/08/17 10:20:54 ptr>

#include "vt_operations.h"

#include <iostream>
#include <janus/vtime.h>

using namespace janus;
using namespace std;

class VTM_handler :
    public stem::EventHandler
{
  public:
    VTM_handler();
    VTM_handler( stem::addr_type id );
    VTM_handler( stem::addr_type id, const char *info );
    ~VTM_handler();

    void handlerE( const stem::Event_base<VSmess>& );
    void handlerV( const VSmess& );

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

void VTM_handler::handlerE( const stem::Event_base<VSmess>& ev )
{
  code = ev.value().code;
  src = ev.value().src;
  gvt = ev.value().gvt;
  grp = ev.value().grp;
  mess = ev.value().mess;

  PushState( 1 );
  cnd.set( true );
}

void VTM_handler::handlerV( const VSmess& m )
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
  EV_Event_base_T_( ST_NULL, VT_MESS, handlerE, VSmess )
  EV_T_( 1, VT_MESS, handlerV, VSmess )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VTMess_core)
{
  const oid_type t0(0);
  const oid_type t1(1);
  const oid_type t3(3);

  VTM_handler h;

  stem::Event_base<VSmess> ev( VT_MESS );

  ev.dest( h.self_id() );
  ev.value().code = 2;
  ev.value().src = t3;
  ev.value().gvt[0][t0] = 1;
  ev.value().gvt[0][t1] = 2;
  ev.value().gvt[1][t0] = 3;
  ev.value().gvt[1][t1] = 4;
  ev.value().grp = janus::vs_base::first_user_group + 7;
  ev.value().mess = "data";

  h.Send( ev );

  h.wait();

  EXAM_CHECK( h.code == 2 );
  EXAM_CHECK( h.src == t3 );
  EXAM_CHECK( h.gvt[0][t0] == 1 );
  EXAM_CHECK( h.gvt[0][t1] == 2 );
  EXAM_CHECK( h.gvt[1][t0] == 3 );
  EXAM_CHECK( h.gvt[1][t1] == 4 );
  EXAM_CHECK( h.grp == (janus::vs_base::first_user_group + 7) );
  EXAM_CHECK( h.mess == "data" );

  ev.value().code = 3;
  ev.value().mess = "more data";

  h.Send( ev );

  h.wait();

  EXAM_CHECK( h.code == 3 );
  EXAM_CHECK( h.src == t3 );
  EXAM_CHECK( h.gvt[0][t0] == 1 );
  EXAM_CHECK( h.gvt[0][t1] == 2 );
  EXAM_CHECK( h.gvt[1][t0] == 3 );
  EXAM_CHECK( h.gvt[1][t1] == 4 );
  EXAM_CHECK( h.grp == (janus::vs_base::first_user_group + 7) );
  EXAM_CHECK( h.mess == "more data" );

  return EXAM_RESULT;
}

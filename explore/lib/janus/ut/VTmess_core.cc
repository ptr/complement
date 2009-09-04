// -*- C++ -*- Time-stamp: <09/09/04 22:52:03 ptr>

/*
 *
 * Copyright (c) 2008-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"

#include <iostream>
#include <janus/vtime.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

namespace janus {

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

    template <class Duration>
    bool wait( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );

        return cnd.timed_wait( lk, rel_time, status );
      }

    stem::code_type code;
    addr_type src;    
    gvtime gvt;
    gid_type grp;
    std::string mess;

    void reset()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); pass = false; }


  private:
    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    bool pass;

    struct _status
    {
        _status( VTM_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_handler& me;
    } status;


    DECLARE_RESPONSE_TABLE( VTM_handler, stem::EventHandler );
};

#define VT_MESS  0x1201

VTM_handler::VTM_handler() :
    EventHandler(),
    pass( false ),
    status( *this )
{
  enable();
}

VTM_handler::VTM_handler( stem::addr_type id ) :
    EventHandler( id ),
    pass( false ),
    status( *this )
{
  enable();
}

VTM_handler::VTM_handler( stem::addr_type id, const char *info ) :
    EventHandler( id, info ),
    pass( false ),
    status( *this )
{
  enable();
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

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

void VTM_handler::handlerV( const VSmess& m )
{
  code = m.code;
  src = m.src;
  gvt = m.gvt;
  grp = m.grp;
  mess = m.mess;

  PopState();

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

bool VTM_handler::_status::operator()() const
{
  return me.pass;
}

DEFINE_RESPONSE_TABLE( VTM_handler )
  EV_Event_base_T_( ST_NULL, VT_MESS, handlerE, VSmess )
  EV_T_( 1, VT_MESS, handlerV, VSmess )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VTMess_core)
{
  const addr_type t0 = xmt::uid();
  const addr_type t1 = xmt::uid();
  const addr_type t3 = xmt::uid();
  const gid_type g = xmt::uid();
  const gid_type g0 = xmt::uid();
  const gid_type g1 = xmt::uid();

  VTM_handler h;

  stem::Event_base<VSmess> ev( VT_MESS );

  ev.dest( h.self_id() );
  ev.value().code = 2;
  ev.value().src = t3;
  ev.value().gvt[g0][t0] = 1;
  ev.value().gvt[g0][t1] = 2;
  ev.value().gvt[g1][t0] = 3;
  ev.value().gvt[g1][t1] = 4;
  ev.value().grp = g;
  ev.value().mess = "data";

  h.Send( ev );

  EXAM_CHECK( h.wait( std::tr2::milliseconds(500) ) );

  h.reset();

  EXAM_CHECK( h.code == 2 );
  EXAM_CHECK( h.src == t3 );
  EXAM_CHECK( h.gvt[g0][t0] == 1 );
  EXAM_CHECK( h.gvt[g0][t1] == 2 );
  EXAM_CHECK( h.gvt[g1][t0] == 3 );
  EXAM_CHECK( h.gvt[g1][t1] == 4 );
  EXAM_CHECK( h.grp == g );
  EXAM_CHECK( h.mess == "data" );

  ev.value().code = 3;
  ev.value().mess = "more data";

  h.Send( ev );

  EXAM_CHECK( h.wait( std::tr2::milliseconds(500) ) );

  h.reset();

  EXAM_CHECK( h.code == 3 );
  EXAM_CHECK( h.src == t3 );
  EXAM_CHECK( h.gvt[g0][t0] == 1 );
  EXAM_CHECK( h.gvt[g0][t1] == 2 );
  EXAM_CHECK( h.gvt[g1][t0] == 3 );
  EXAM_CHECK( h.gvt[g1][t1] == 4 );
  EXAM_CHECK( h.grp == g );
  EXAM_CHECK( h.mess == "more data" );

  return EXAM_RESULT;
}

} // namespace janus


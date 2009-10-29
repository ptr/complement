// -*- C++ -*- Time-stamp: <09/10/29 11:06:43 ptr>

/*
 *
 * Copyright (c) 2009
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

#include <algorithm>
#include <set>
#include <list>

#include <fstream>
#include <mt/uid.h>
#include <unistd.h>

namespace janus {

using namespace std;

class VTM_one_group_handler :
    public basic_vs
{
  public:
    VTM_one_group_handler();
    VTM_one_group_handler( stem::addr_type id );
    VTM_one_group_handler( stem::addr_type id, const char *info );
    ~VTM_one_group_handler();

    template <class Duration>
    bool wait( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );

        return cnd.timed_wait( lk, rel_time, status );
      }

    vtime_matrix_type& vt()
      { return basic_vs::vt; }

    virtual xmt::uuid_type vs_pub_recover();
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    virtual void vs_pub_view_update();
    virtual void vs_event_origin( const janus::vtime&, const stem::Event& );
    virtual void vs_event_derivative( const vtime&, const stem::Event& );
    virtual void vs_pub_flush();

    std::string mess;

    void reset()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); pass = false; }

  private:
    void message( const stem::Event& );

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    bool pass;

    struct _status
    {
        _status( VTM_one_group_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_handler& me;
    } status;


    DECLARE_RESPONSE_TABLE( VTM_one_group_handler, janus::basic_vs );
};

#define EV_FREE      0x9000

VTM_one_group_handler::VTM_one_group_handler() :
    basic_vs(),
    pass( false ),
    status( *this )
{
  enable();
}

VTM_one_group_handler::VTM_one_group_handler( stem::addr_type id ) :
    basic_vs( id ),
    pass( false ),
    status( *this )
{
  enable();
}

VTM_one_group_handler::VTM_one_group_handler( stem::addr_type id, const char* info ) :
    basic_vs( id, info ),
    pass( false ),
    status( *this )
{
  enable();
}

VTM_one_group_handler::~VTM_one_group_handler()
{
  disable();
}

xmt::uuid_type VTM_one_group_handler::vs_pub_recover()
{
  return xmt::nil_uuid;
}

void VTM_one_group_handler::vs_resend_from( const xmt::uuid_type&, const stem::addr_type& )
{
}

void VTM_one_group_handler::vs_pub_view_update()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

void VTM_one_group_handler::vs_event_origin( const vtime&, const stem::Event& )
{
}

void VTM_one_group_handler::vs_event_derivative( const vtime&, const stem::Event& )
{
}

void VTM_one_group_handler::vs_pub_flush()
{
}

bool VTM_one_group_handler::_status::operator()() const
{
  return me.pass;
}

void VTM_one_group_handler::message( const stem::Event& ev )
{
  mess = ev.value();

  EXAM_CHECK_ASYNC( (ev.flags() & stem::__Event_Base::vs) != 0 );

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( VTM_one_group_handler )
  EV_EDS( ST_NULL, EV_FREE, message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VT_one_group_core)
{
  VTM_one_group_handler a1;
  VTM_one_group_handler a2;
  
  EXAM_CHECK( a1.vs_group_size() == 0 );

  a2.vs_join( a1.self_id() );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a1.vs_group_size() == 2 );

  EXAM_CHECK( a1.vt()[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt()[a1.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a1.vt()[a2.self_id()][a1.self_id()] == 0 );
  EXAM_CHECK( a1.vt()[a2.self_id()][a2.self_id()] == 0 );

  EXAM_CHECK( a2.vt()[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt()[a1.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a2.vt()[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt()[a2.self_id()][a2.self_id()] == 0 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_core3)
{
  VTM_one_group_handler a1;
  VTM_one_group_handler a2;
  VTM_one_group_handler a3;

  a2.vs_join( a1.self_id() );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a1.vt()[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt()[a1.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a1.vt()[a2.self_id()][a1.self_id()] == 0 );
  EXAM_CHECK( a1.vt()[a2.self_id()][a2.self_id()] == 0 );

  EXAM_CHECK( a2.vt()[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt()[a1.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a2.vt()[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt()[a2.self_id()][a2.self_id()] == 0 );

  a1.reset();
  a2.reset();

  a3.vs_join( a2.self_id() );

  EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );
  EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a1.vt()[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt()[a1.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a1.vt()[a1.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a1.vt()[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a1.vt()[a2.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a1.vt()[a2.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a1.vt()[a3.self_id()][a1.self_id()] == 0 );
  EXAM_CHECK( a1.vt()[a3.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a1.vt()[a3.self_id()][a3.self_id()] == 0 );

  EXAM_CHECK( a2.vt()[a1.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt()[a1.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a2.vt()[a1.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a2.vt()[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a2.vt()[a2.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a2.vt()[a2.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a2.vt()[a3.self_id()][a1.self_id()] == 0 );
  EXAM_CHECK( a2.vt()[a3.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a2.vt()[a3.self_id()][a3.self_id()] == 0 );

  EXAM_CHECK( a3.vt()[a1.self_id()][a1.self_id()] == 0 );
  EXAM_CHECK( a3.vt()[a1.self_id()][a2.self_id()] == 0 );
  EXAM_CHECK( a3.vt()[a1.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a3.vt()[a2.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a3.vt()[a2.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a3.vt()[a2.self_id()][a3.self_id()] == 0 );
  EXAM_CHECK( a3.vt()[a3.self_id()][a1.self_id()] == 1 );
  EXAM_CHECK( a3.vt()[a3.self_id()][a2.self_id()] == 2 );
  EXAM_CHECK( a3.vt()[a3.self_id()][a3.self_id()] == 0 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_send)
{
  VTM_one_group_handler a1;
  VTM_one_group_handler a2;

  a2.vs_join( a1.self_id() );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  a1.reset();
  a2.reset();

  stem::Event ev( EV_FREE );
  ev.value() = "message";

  a1.vs( ev );
  EXAM_CHECK( a1.vt()[a1.self_id()][a1.self_id()] == 2 );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a2.vt()[a2.self_id()][a1.self_id()] == 2 );
  EXAM_CHECK( a2.mess == "message" );

  a1.reset();
  a2.reset();

  VTM_one_group_handler a3;

  a3.vs_join( a1.self_id() );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
  EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a3.vt()[a3.self_id()][a1.self_id()] == 4 );
  EXAM_CHECK( a3.vt()[a1.self_id()][a1.self_id()] == 4 );
  // EXAM_CHECK( a3.vt()[a2.self_id()][a1.self_id()] == 1 );

  a1.reset();
  a2.reset();
  a3.reset();

  ev.value() = "another message";

  a3.vs( ev );
  EXAM_CHECK( a3.vt()[a3.self_id()][a3.self_id()] == 1 );

  EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a2.vt()[a2.self_id()][a3.self_id()] == 1 );
  EXAM_CHECK( a2.mess == "another message" );

  EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );

  EXAM_CHECK( a1.vt()[a1.self_id()][a3.self_id()] == 1 );
  EXAM_CHECK( a1.mess == "another message" );

  return EXAM_RESULT;
}

} // namespace janus

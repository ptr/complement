// -*- C++ -*- Time-stamp: <2012-02-08 19:17:33 ptr>

/*
 *
 * Copyright (c) 2009-2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"

#include <iostream>
#include <janus/causal.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

#include <algorithm>
#include <set>
#include <list>

#include <fstream>
#include <mt/uid.h>
#include <unistd.h>

#include <stem/EvManager.h>

namespace janus {

using namespace std;

class VTM_one_group_handler :
    public basic_vs
{
  public:
    VTM_one_group_handler();
    ~VTM_one_group_handler();

    template <class Duration>
    bool wait_group_size( const Duration& rel_time, int _gsize )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        
        gsize = _gsize;
        
        return cnd.timed_wait( lk, rel_time, gs_status );
      }

    template <class Duration>
    bool wait_msg( const Duration& rel_time, int _n_msg )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        
        n_msg = _n_msg;

        return cnd.timed_wait( lk, rel_time, msg_status );
      }

    template <class Duration>
    bool wait_flush( const Duration& rel_time, int _n_flush )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        
        n_flush = _n_flush;

        return cnd.timed_wait( lk, rel_time, flush_status );
      }

    template <class Duration>
    bool wait_join( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        return cnd.timed_wait( lk, rel_time, join_status );
      }

    vtime& vt()
      { return basic_vs::vt; }
    
    virtual xmt::uuid_type vs_pub_recover( bool is_fouder );
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::ext_addr_type& );
    virtual void vs_pub_view_update();
    virtual void vs_pub_rec( const stem::Event& );
    virtual void vs_pub_join();

    virtual void vs_pub_flush();

    virtual std::tr2::milliseconds vs_pub_lock_timeout() const
      { return std::tr2::milliseconds( 250 ); }

    std::string mess;

    void reset()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); msg = 0; }

    void reset_flush()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); flush = 0; }

  private:
    void message( const stem::Event& );

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    int n_msg;
  public:
    int msg;
  private:
    int n_flush;
    int flush;
    int gsize;
    bool joined;

    struct _gs_status
    {
        _gs_status( VTM_one_group_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_handler& me;
    } gs_status;    

    struct _msg_status
    {
        _msg_status( VTM_one_group_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_handler& me;
    } msg_status;

    struct _flush_status
    {
        _flush_status( VTM_one_group_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_handler& me;
    } flush_status;

    struct _join_status
    {
        _join_status( VTM_one_group_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_handler& me;
    } join_status;

    DECLARE_RESPONSE_TABLE( VTM_one_group_handler, janus::basic_vs );
};

#define EV_FREE      0x9000

VTM_one_group_handler::VTM_one_group_handler() :
    basic_vs(),
    msg_status( *this ),
    gs_status( *this ),
    flush_status( *this ),
    join_status( *this ),
    msg(0),
    flush(0),
    joined(false)
{
  enable();
}

VTM_one_group_handler::~VTM_one_group_handler()
{
  disable();
}

xmt::uuid_type VTM_one_group_handler::vs_pub_recover( bool is_founder )
{
  return xmt::nil_uuid;
}

void VTM_one_group_handler::vs_resend_from( const xmt::uuid_type&, const stem::ext_addr_type& )
{
}

void VTM_one_group_handler::vs_pub_view_update()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  cnd.notify_one();
}

void VTM_one_group_handler::vs_pub_rec( const stem::Event& )
{
}

void VTM_one_group_handler::vs_pub_flush()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  ++flush;
  cnd.notify_one();
}

void VTM_one_group_handler::vs_pub_join()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  joined = true;
  cnd.notify_one();
}

bool VTM_one_group_handler::_join_status::operator()() const
{
  return me.joined;
}

bool VTM_one_group_handler::_flush_status::operator()() const
{
  return me.flush >= me.n_flush;
}

bool VTM_one_group_handler::_msg_status::operator()() const
{
  return me.msg == me.n_msg;
}

bool VTM_one_group_handler::_gs_status::operator()() const
{
  return me.vs_group_size() == me.gsize;
}

void VTM_one_group_handler::message( const stem::Event& ev )
{
  mess = ev.value();

  EXAM_CHECK_ASYNC( (ev.flags() & stem::__Event_Base::vs) != 0 );

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  // stringstream ss(mess);
  ++msg;
  cnd.notify_one();
}


DEFINE_RESPONSE_TABLE( VTM_one_group_handler )
  EV_EDS( ST_NULL, EV_FREE, message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VT_one_group_core)
{
  for (int i = 0;i < 1000;++i) {
    // misc::use_syslog<LOG_INFO,LOG_USER>() << "-----------------" << endl;
  VTM_one_group_handler a1;

  a1.manager().settrs( &cerr );
  a1.manager().settrf( stem::EvManager::tracedispatch | stem::EvManager::tracetime | stem::EvManager::tracefault | stem::EvManager::tracesend );

  EXAM_CHECK( a1.vs_group_size() == 0 );

  // join to itself is prohibited
  EXAM_CHECK( a1.vs_join(make_pair(stem::EventHandler::domain(), a1.self_id()) ) == 1 );
  EXAM_CHECK( a1.vs_group_size() == 0 );

  a1.vs_join( stem::extbadaddr );
  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 1 ) );

  {
    VTM_one_group_handler a2;

    a2.vs_join( make_pair( stem::EventHandler::domain(), a1.self_id()) );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a2.wait_join( std::tr2::milliseconds(500) ) );
  }

  // we can detect that a2 leave group after event (with delay)
  // or after flush message:
  a1.vs_send_flush();

  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 1 ) );
  if ( EXAM_RESULT ) {
    break;
  }
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_core3)
{
  for (int i = 0;i < 1000;++i) {
  VTM_one_group_handler a1;
  VTM_one_group_handler a2;
  VTM_one_group_handler a3;

  a1.vs_join( stem::extbadaddr );
  a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );
  
  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.wait_join( std::tr2::milliseconds(500)) );

  a3.vs_join( make_pair(stem::EventHandler::domain(), a2.self_id()) );

  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
  EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
  EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3 ) );
  EXAM_CHECK( a3.wait_join( std::tr2::milliseconds(500)) );
  if ( EXAM_RESULT ) {
    break;
  }
  }
  
  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_core3_sim)
{
  VTM_one_group_handler a0;

  // a0.manager()->settrs( &cerr );
  // a0.manager()->settrf( stem::EvManager::tracedispatch | stem::EvManager::tracetime | stem::EvManager::tracefault | stem::EvManager::tracesend );

  for ( int i = 0; i < 1000; ++i ) {
    VTM_one_group_handler a1;
    VTM_one_group_handler a2;
    VTM_one_group_handler a3;

    // misc::use_syslog<LOG_INFO,LOG_USER>() << "-----------------" << endl;
    // misc::use_syslog<LOG_INFO,LOG_USER>() << "a1 = " << a1.self_id() << endl;
    // misc::use_syslog<LOG_INFO,LOG_USER>() << "a2 = " << a2.self_id() << endl;
    // misc::use_syslog<LOG_INFO,LOG_USER>() << "a3 = " << a3.self_id() << endl;

    a1.vs_join( stem::extbadaddr );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 1 ) );

    a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );
    a3.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a1.wait_join( std::tr2::milliseconds(500) ) );
    EXAM_CHECK( a2.wait_join( std::tr2::milliseconds(500) ) );

    VTM_one_group_handler* a4 = new VTM_one_group_handler();
  
    a4->vs_join( make_pair(stem::EventHandler::domain(), a3.self_id()) );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 4 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 4 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 4 ) );
    EXAM_CHECK( a4->wait_group_size( std::tr2::milliseconds(500), 4 ) );
  
    delete a4;
  
    a1.vs_send_flush();

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(5000), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(5000), 3 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(5000), 3 ) );

    if ( EXAM_RESULT ) {
      stringstream s;
      s << "on iteration " << i;
      EXAM_ERROR( s.str().c_str() );
      break;
    }
  }
  
  return EXAM_RESULT;
}


int EXAM_IMPL(vtime_operations::VT_one_group_join_exit)
{
  for (int i = 0;i < 1000;++i) {
  VTM_one_group_handler a1;
  VTM_one_group_handler a2;

  a1.vs_join( stem::extbadaddr );

  {
    VTM_one_group_handler a3;
    a3.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 2 ) );

    a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );
  }

  a1.vs_send_flush();

  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::double_flush)
{
  VTM_one_group_handler a0;

  for ( int i = 0; i < 1000; ++i ) {
    VTM_one_group_handler a1;
    VTM_one_group_handler a2;
    VTM_one_group_handler a3;
  
    a1.vs_join( stem::extbadaddr );
    a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );
    a3.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3) );

    a1.vs_send_flush();
    a2.vs_send_flush();

    EXAM_CHECK( a1.wait_flush( std::tr2::milliseconds(500), 2) );
    EXAM_CHECK( a2.wait_flush( std::tr2::milliseconds(500), 2) );
    EXAM_CHECK( a3.wait_flush( std::tr2::milliseconds(500), 2) );

    VTM_one_group_handler a4;

    a4.vs_join( make_pair(stem::EventHandler::domain(), a3.self_id()) );
    EXAM_CHECK( a4.wait_group_size( std::tr2::milliseconds(500), 4) );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::double_exit)
{
  VTM_one_group_handler a0;

  for ( int i = 0; i < 1000; ++i ) {
    VTM_one_group_handler a1;

    a1.vs_join( stem::extbadaddr );

    {
      VTM_one_group_handler a2;
      VTM_one_group_handler a3;

      a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );
      a3.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

      EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3) );
      EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3) );
      EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3) );
    }

    VTM_one_group_handler a4;

    a4.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );
    EXAM_CHECK( a4.wait_group_size( std::tr2::milliseconds(500), 2) );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::flush_and_join)
{
  VTM_one_group_handler a0;

  for ( int i = 0; i < 1000; ++i ) {
    VTM_one_group_handler a1;
    VTM_one_group_handler a2;
    VTM_one_group_handler a3;

    a1.vs_join( stem::extbadaddr );
    a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2) );

    a3.vs_join( make_pair(stem::EventHandler::domain(), a2.self_id()) );
    a1.vs_send_flush();

    EXAM_CHECK( a1.wait_flush( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a2.wait_flush( std::tr2::milliseconds(500), 1) );
    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3) );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::flush_and_exit)
{
  VTM_one_group_handler a0;

  for ( int i = 0; i < 1000; ++i ) {
    VTM_one_group_handler a2;
    VTM_one_group_handler a3;

    {
      VTM_one_group_handler a1;

      a1.vs_join( stem::extbadaddr );
      a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

      EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2) );
      EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2) );

      a1.vs_send_flush();
    }

    a2.vs_send_flush();
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 1) );

    a3.vs_join( make_pair(stem::EventHandler::domain(), a2.self_id()) );

    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 2) );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::join_flush_exit)
{
  for ( int i = 0; i < 100; ++i ) {
    VTM_one_group_handler a1;
    VTM_one_group_handler a2;
    VTM_one_group_handler a4;

    {
      VTM_one_group_handler a3;
      misc::use_syslog<LOG_INFO,LOG_USER>() << "-----------------" << endl;
      misc::use_syslog<LOG_INFO,LOG_USER>() << "a1 = " << a1.self_id() << endl;
      misc::use_syslog<LOG_INFO,LOG_USER>() << "a2 = " << a2.self_id() << endl;
      misc::use_syslog<LOG_INFO,LOG_USER>() << "a3 = " << a3.self_id() << endl;
      misc::use_syslog<LOG_INFO,LOG_USER>() << "a4 = " << a4.self_id() << endl;

      a1.vs_join( stem::extbadaddr );
      a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );
      a3.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

      EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3) );
      EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3) );

      a1.vs_send_flush();
      a4.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );
    }
  
    a1.vs_send_flush();
    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3) );
    EXAM_CHECK( a4.wait_group_size( std::tr2::milliseconds(500), 3) );

    EXAM_CHECK( a1.wait_flush( std::tr2::milliseconds(500), 2) );
    EXAM_CHECK( a2.wait_flush( std::tr2::milliseconds(500), 2) );
    if ( EXAM_RESULT ) {
      break;
    }
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_send)
{
  for ( int i = 0; i < 1000; ++i ) {
    misc::use_syslog<LOG_INFO,LOG_USER>() << "------------ " << endl;
    VTM_one_group_handler a1;
    VTM_one_group_handler a2;

    a1.vs_join( stem::extbadaddr );
    a2.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

    stem::Event ev( EV_FREE );
    ev.value() = "message";

    a1.vs( ev );

    EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 1 ) );

    EXAM_CHECK( a1.mess == "message" );
    EXAM_CHECK( a2.mess == "message" );

    VTM_one_group_handler a3;

    a3.vs_join( make_pair(stem::EventHandler::domain(), a1.self_id()) );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3 ) );  

    ev.value() = "another message";

    a3.vs( ev );

    EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a3.wait_msg( std::tr2::milliseconds(500), 1 ) );

    EXAM_CHECK( a1.mess == "another message" );
    EXAM_CHECK( a2.mess == "another message" );
    EXAM_CHECK( a3.mess == "another message" );
    if ( EXAM_RESULT ) {
      break;
    }
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_multiple_send)
{
  int n_obj = 10;
  int n_msg = 1000;
  srand( time(NULL) );
  vector<VTM_one_group_handler*> a(n_obj);
  

  // cerr << HERE << ' ' << std::tr2::get_system_time().nanoseconds_since_epoch().count() << endl;
  for ( int i = 0; i < n_obj; ++i ) {
    a[i] = new VTM_one_group_handler;
  }

  // cerr << HERE << ' ' << std::tr2::get_system_time().nanoseconds_since_epoch().count() << endl;

  a[0]->vs_join( stem::extbadaddr );

  stem::addr_type addr = a[0]->self_id();
  for ( int i = 1; i < n_obj; ++i ) {
    a[i]->vs_join( make_pair(stem::EventHandler::domain(), addr) );
  }

  // cerr << HERE << ' ' << std::tr2::get_system_time().nanoseconds_since_epoch().count() << endl;

  for ( int i = 0; i < n_obj; ++i ) {
    EXAM_CHECK( a[i]->wait_group_size(std::tr2::milliseconds(n_obj * 100), n_obj) );
  }

  // cerr << HERE << ' ' << std::tr2::get_system_time().nanoseconds_since_epoch().count() << endl;

  stem::Event ev( EV_FREE );
  ev.value() = "message";

  for ( int i = 0; i < n_msg; ++i ) {
    a[rand() % n_obj]->vs( ev );
  }

  // cerr << HERE << ' ' << std::tr2::get_system_time().nanoseconds_since_epoch().count() << endl;

  for ( int i = 0; i < n_obj; ++i ) {
    // EXAM_CHECK( a[i]->wait_msg( std::tr2::milliseconds(n_msg * 20), n_msg ) );
    if ( !a[i]->wait_msg( std::tr2::milliseconds(n_msg * 20), n_msg ) ) {
      stringstream s;

      s << HERE << ' ' << i << ' ' << a[i]->msg << " (" << n_msg << ")" << endl;
      EXAM_ERROR( s.str().c_str() );
    }
  }

  for ( int i = 0; i < n_obj; ++i ) {
    delete a[i];
    for ( int j = i + 1; j < n_obj; ++j ) {
      a[j]->vs_send_flush();
      EXAM_CHECK( a[j]->wait_group_size(std::tr2::milliseconds(n_obj * 100), n_obj - i - 1) );
    }
  }

  return EXAM_RESULT;
}

namespace mt_operation {

const int n_obj = 4;
const int n_msg = 1;
std::tr2::thread* thr[n_obj];
int res[n_obj];
stem::ext_addr_type addr; 

void run(int k)
{
  VTM_one_group_handler a;
  a.vs_join( addr );
  EXAM_CHECK_ASYNC( a.wait_group_size( std::tr2::milliseconds( max(500, n_obj * 200)), n_obj + 1 ) );

  stem::Event ev( EV_FREE );
  ev.value() = "message";

  for (int i = 0;i < n_msg;++i) {
    a.vs( ev );
  }
  a.vs_send_flush();

  EXAM_CHECK_ASYNC_F( a.wait_msg( std::tr2::milliseconds( max(500, n_msg * n_obj * 20)), n_obj * n_msg ), res[k] );
  EXAM_CHECK_ASYNC_F( a.wait_flush( std::tr2::milliseconds( max(500, n_obj * 200)), n_obj ), res[k] );
}

};

int EXAM_IMPL(vtime_operations::VT_mt_operation)
{
  for (int i = 0;i < 100;++i) {
  VTM_one_group_handler a;
  mt_operation::addr = make_pair(stem::EventHandler::domain(), a.self_id());

  misc::use_syslog<LOG_INFO,LOG_USER>() << "----------------- " << mt_operation::addr.second << endl;

  a.vs_join( stem::extbadaddr );

  for (int i = 0;i < mt_operation::n_obj;++i) {
    mt_operation::thr[i] = new std::tr2::thread( mt_operation::run, i );
  }

  for (int i = 0;i < mt_operation::n_obj;++i) {
    mt_operation::thr[i]->join();
    delete mt_operation::thr[i];
    EXAM_CHECK( mt_operation::res[i] == 0 );
  }

  EXAM_CHECK( a.wait_msg( std::tr2::milliseconds( max(500, mt_operation::n_msg * mt_operation::n_obj * 20)), mt_operation::n_obj * mt_operation::n_msg ) );
  EXAM_CHECK( a.wait_flush( std::tr2::milliseconds( max(500, mt_operation::n_obj * 200)), mt_operation::n_obj ) );
  if ( EXAM_RESULT ) {
    break;
  }
  }

  return EXAM_RESULT;
}


} // namespace janus

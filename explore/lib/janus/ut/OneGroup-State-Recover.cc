// -*- C++ -*- Time-stamp: <10/07/12 12:07:17 ptr>

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
#include <janus/casual.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>
#include <sys/wait.h>
#include <mt/shm.h>
#include <mt/thread>
#include <sockios/sockmgr.h>
#include <stem/NetTransport.h>

#include <algorithm>
#include <set>
#include <list>

#include <fstream>
#include <mt/uid.h>
#include <unistd.h>

#include <sockios/syslog.h>

#ifndef VS_FLUSH_RQ
# define VS_FLUSH_RQ 0x307 // see casual.cc
#endif

namespace janus {

using namespace std;

#define EV_FREE      0x9000
#define EV_FREE_SYNC 0x9001

class VTM_one_group_recover :
    public basic_vs
{
  public:
    VTM_one_group_recover();
    VTM_one_group_recover( const stem::addr_type& id );
    ~VTM_one_group_recover();

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
    bool wait_flush( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        return cnd.timed_wait( lk, rel_time, flush_status );
      }

    vtime& vt()
      { return basic_vs::vt; }

    virtual xmt::uuid_type vs_pub_recover();
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    virtual void vs_pub_view_update();
    virtual void vs_pub_rec( const stem::Event& );
    virtual void vs_pub_flush();

    std::string mess;

    void reset_msg()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); msg = 0; }

    void reset_flush()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); flushed = false; }

  private:
    void message( const stem::Event& );
    void sync_message( const stem::Event& );

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    int n_msg;
    int msg;
    int gsize;
    bool flushed;

    struct _gs_status
    {
        _gs_status( VTM_one_group_recover& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_recover& me;
    } gs_status;    

    struct _msg_status
    {
        _msg_status( VTM_one_group_recover& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_recover& me;
    } msg_status;

    struct _flush_status
    {
        _flush_status( VTM_one_group_recover& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_recover& me;
    } flush_status;


    std::fstream history;

    DECLARE_RESPONSE_TABLE( VTM_one_group_recover, janus::basic_vs );
};

VTM_one_group_recover::VTM_one_group_recover() :
    basic_vs(),
    msg_status( *this ),
    gs_status( *this ),
    flush_status( *this ),
    msg(0),
    flushed(false)
{
  string nm( "/tmp/janus." );
  nm += std::string( self_id() );

  history.open( nm.c_str(), ios_base::in | ios_base::out /* | ios_base::app */ );
  if ( !history.is_open() ) {
    history.clear();
    history.open( nm.c_str(), ios_base::in | ios_base::out | ios_base::trunc );
    uint64_t last_flush_off = 0;
    stem::__pack_base::__pack( history, last_flush_off );
  }

  enable();
}

VTM_one_group_recover::VTM_one_group_recover( const stem::addr_type& id ) :
    basic_vs(),
    msg_status( *this ),
    gs_status( *this ),
    flush_status( *this ),
    msg(0),
    flushed(false)
{
  string nm( "/tmp/janus." );
  nm += std::string( id );

  history.open( nm.c_str(), ios_base::in | ios_base::out /* | ios_base::app */ );
  if ( !history.is_open() ) {
    history.clear();
    history.open( nm.c_str(), ios_base::in | ios_base::out | ios_base::trunc );
    uint64_t last_flush_off = 0;
    stem::__pack_base::__pack( history, last_flush_off );
  }

  enable();
}

VTM_one_group_recover::~VTM_one_group_recover()
{
  disable();
}

bool VTM_one_group_recover::_msg_status::operator()() const
{
  return me.msg == me.n_msg;
}

bool VTM_one_group_recover::_gs_status::operator()() const
{
  return me.vs_group_size() == me.gsize;
}

bool VTM_one_group_recover::_flush_status::operator()() const
{
  return me.flushed;
}

xmt::uuid_type VTM_one_group_recover::vs_pub_recover()
{
  stem::Event ev;
  stem::code_type c;
  uint32_t f;
  xmt::uuid_type flush_id = xmt::nil_uuid;
  ev.dest( self_id() );
  ev.src( stem::badaddr );

  try {
    // Try to read serialized events and re-play history.
    // Note: don't call this function from ctor,
    // because
    //   - it _virtual_ function
    //   - it call 'replay' that call StEM's Dispatch,
    //     and Dispatch is virtual too, and processing
    //     depend upon data from this class.
    history.seekg( 0, ios_base::beg );
    uint64_t last_flush_off = 0;

    stem::__pack_base::__unpack( history, last_flush_off );
    if ( !history.fail() ) { // offset for last flush ok
      while ( !history.fail() ) {
        stem::__pack_base::__unpack( history, c );
        ev.code( c );
        stem::__pack_base::__unpack( history, f );
        ev.resetf( f );
        stem::__pack_base::__unpack( history, ev.value() );

        if ( !history.fail() ) {
          if ( history.tellg() <= last_flush_off ) {
            if ( ev.code() == VS_FLUSH_RQ ) {
              stem::Event_base<xmt::uuid_type> fev;
              fev.unpack( ev );              
              flush_id = fev.value();
            } else {
              // basic_vs::sync_call( ev );
              this->Dispatch( ev );
            }
          } else {
            break;
          }
        }
      }
      history.clear();
      history.seekp( max( last_flush_off, static_cast<uint64_t>(sizeof(last_flush_off)) ), ios_base::beg );
    } else { // can't recover offset after last flush
      history.clear();
      history.seekp( 0, ios_base::beg );
      last_flush_off = 0;
      stem::__pack_base::__pack( history, last_flush_off );
      flush_id = xmt::nil_uuid;
    }
  }
  catch ( const std::runtime_error& err ) {
    history.clear();
    history.seekp( 0, ios_base::end );
    flush_id = xmt::nil_uuid;
  }

  return flush_id;
}

void VTM_one_group_recover::vs_resend_from( const xmt::uuid_type& from, const stem::addr_type& addr )
{
  if ( !is_avail( addr ) ) {
    return;
  }

  stem::Event ev;
  stem::code_type c;
  uint32_t f;
  bool ref_point_found = (from == xmt::nil_uuid) ? true : false;

  ev.dest( addr );
  ev.src( self_id() );

  history.seekg( sizeof(uint64_t), ios_base::beg );

  while ( !history.fail() ) {
    stem::__pack_base::__unpack( history, c );
    ev.code( c );
    stem::__pack_base::__unpack( history, f );
    ev.resetf( f );
    stem::__pack_base::__unpack( history, ev.value() );

    if ( !history.fail() ) {
      if ( !ref_point_found ) {
        if ( ev.code() == VS_FLUSH_RQ ) {
          stem::Event_base<xmt::uuid_type> fev;
          fev.unpack( ev );              
          if ( fev.value() == from ) {
            ref_point_found = true;
          }
        }
      } else {
        /* flag stem::__Event_Base::vs is set to avoid forwarding
           'recovery' event to other group members;
           stem::__Event_Base::vs_join is set to distinguish event
           as come during recovery and record it (see VTM_one_group_recover::message,
           and call VTM_one_group_recover::vs_event_derivative in
           VTM_one_group_recover::message).
         */
        ev.setf( stem::__Event_Base::vs | stem::__Event_Base::vs_join );
        // Change event, to avoid interference with
        // true VS_FLUSH_VIEW---it work with view lock,
        // but in this case I want to bypass locking
        if ( ev.code() != VS_FLUSH_RQ ) {
          Forward( ev ); // src was set above
        }
      }
    }
  }

  history.clear();
  // history.seekp( 0, ios_base::end );
}

void VTM_one_group_recover::vs_pub_view_update()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  cnd.notify_one();
}

void VTM_one_group_recover::vs_pub_rec( const stem::Event& ev )
{
  stem::__pack_base::__pack( history, ev.code() );
  stem::__pack_base::__pack( history, ev.flags() );
  stem::__pack_base::__pack( history, ev.value() );
}

void VTM_one_group_recover::vs_pub_flush()
{
  uint64_t last_flush_off = history.tellp();
  history.seekp( 0, ios_base::beg );
  stem::__pack_base::__pack( history, last_flush_off );
  history.seekp( 0, ios_base::end );

  flushed = true;

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  cnd.notify_one();
}


void VTM_one_group_recover::message( const stem::Event& ev )
{
  stem::Event sync( ev );
  sync.code( EV_FREE_SYNC );

  vs( sync );
}

void VTM_one_group_recover::sync_message( const stem::Event& ev )
{
  mess = ev.value();

  if ( (ev.flags() & stem::__Event_Base::vs_join) != 0 ) {
    // This is event come during recovery procedure:
    stem::Event xev;
    ev.pack( xev );
    VTM_one_group_recover::vs_pub_rec( xev );
  }

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  ++msg;
  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( VTM_one_group_recover )
  EV_EDS( ST_NULL, EV_FREE, message )
  EV_EDS( ST_NULL, EV_FREE_SYNC, sync_message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VT_one_group_recover)
{
  stem::addr_type a1_stored;
  stem::addr_type a2_stored;
  stem::addr_type a3_stored;

  VTM_one_group_recover a1;
  VTM_one_group_recover a2;

  a1_stored = a1.self_id();

  // first, but join required for vs_pub_recover:
  a1.vs_join( stem::badaddr );

  a2_stored = a2.self_id();

  a2.vs_join( a1.self_id() );

  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

  {
    VTM_one_group_recover a3;

    a3_stored = a3.self_id();

    a3.vs_join( a1.self_id() );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3 ) );
  
    stem::Event ev( EV_FREE );
    ev.value() = "message";
    ev.dest( a1.self_id() );

    a1.Send( ev );

    EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a1.mess == "message" );
    EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a2.mess == "message" );
    EXAM_CHECK( a3.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a3.mess == "message" );

    a1.vs_send_flush();

    EXAM_CHECK( a1.wait_flush( std::tr2::milliseconds(500) ) );
    EXAM_CHECK( a2.wait_flush( std::tr2::milliseconds(500) ) );
    EXAM_CHECK( a3.wait_flush( std::tr2::milliseconds(500) ) );
  }

  a1.vs_send_flush();

  {
    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

    stem::Event ev( EV_FREE );
    ev.value() = "extra message";
    ev.dest( a1.self_id() );

    a1.Send( ev );

    EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a1.mess == "extra message" );
    EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a2.mess == "extra message" );
  }

  {
    VTM_one_group_recover a3( a3_stored );

    a3.vs_join( a2.self_id() );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3 ) );

    EXAM_CHECK( a3.wait_msg( std::tr2::milliseconds(500), 2 ) );

    EXAM_CHECK( a3.mess == "extra message" );
  }

  unlink( (std::string( "/tmp/janus." ) + std::string(a1_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a2_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a3_stored) ).c_str() );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_join_send)
{
  stem::addr_type a1_stored;
  stem::addr_type a2_stored;
  stem::addr_type a3_stored;

  VTM_one_group_recover a1;
  VTM_one_group_recover a2;

  a1_stored = a1.self_id();

  a1.vs_join( stem::badaddr );
  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 1 ) );

  a2_stored = a2.self_id();

  a2.vs_join( a1.self_id() );

  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

  {
    VTM_one_group_recover a3;

    a3_stored = a3.self_id();

    stem::Event ev( EV_FREE );
    ev.value() = "message";
    ev.dest( a1.self_id() );

    a3.vs_join( a2.self_id() );
    a1.Send( ev );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3 ) );

    EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a1.mess == "message" );
    EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a2.mess == "message" );
    EXAM_CHECK( a3.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a3.mess == "message" );

    ev.value() = "another message";
    a1.Send( ev );
  }

  a1.vs_send_flush();

  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

  EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a1.mess == "another message" );
  EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.mess == "another message" );

  unlink( (std::string( "/tmp/janus." ) + std::string(a1_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a2_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a3_stored) ).c_str() );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_multiple_joins)
{
  const int n = 10;
  vector< stem::addr_type > names(n);

  try {
    srand( time(NULL) );

    vector<VTM_one_group_recover*> a(n);

    for ( int i = 0; i < n; ++i ) {
      a[i] = new VTM_one_group_recover();
      names[i] = a[i]->self_id();
    }

    a[0]->vs_join( stem::badaddr );

    EXAM_CHECK( a[0]->vs_group_size() == 1 );

    for ( int i = 1; i < n; ++i ) {
      int p = rand() % i;
      int q = rand() % i;

      stem::Event ev( EV_FREE );
      stringstream ss;
      ss << i;
      ev.value() = ss.str();
      ev.dest( a[p]->self_id() );

      a[i]->vs_join( a[q]->self_id() );
      a[p]->Send( ev );

      for (int j = 0; j <= i; ++j ) {
        EXAM_CHECK( a[j]->wait_group_size( std::tr2::milliseconds( (i + 1) * 200), i + 1 ) );
        EXAM_CHECK( a[j]->wait_msg( std::tr2::milliseconds( (i + 1) * 200 ), i ) );
        EXAM_CHECK( a[j]->mess == ss.str() );
      }
    }

    for ( int i = 0; i < n; ++i ) {
      delete a[i];
    }
  }
  catch ( const std::runtime_error& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( std::exception& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( ... ) {
    EXAM_ERROR( "unknown exception" );
  }

  for ( int i = 0; i < n; ++i ) {
    unlink( (std::string( "/tmp/janus." ) + std::string(names[i]) ).c_str() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_multiple_join_send)
{
  const int n = 10;
  vector< stem::addr_type > names(n);

  try {
    srand( time(NULL) );

    vector<VTM_one_group_recover*> a(n);

    for ( int i = 0; i < n; ++i ) {
      a[i] = new VTM_one_group_recover();
      names[i] = a[i]->self_id();
    }

    a[0]->vs_join( stem::badaddr );

    EXAM_CHECK( a[0]->vs_group_size() == 1 );

    int k = 0;
    for ( int i = 1; i < n; ++i ) {
      a[i]->vs_join( a[i - 1]->self_id() );
      for (int j = 0;j < i;++j) {
        stem::Event ev( EV_FREE );
        ev.dest( a[j]->self_id() );
        a[j]->Send( ev );
      }
      a[i - 1]->vs_send_flush();
      k += i;
      EXAM_CHECK( a[i]->wait_group_size( std::tr2::milliseconds(n * 200), i + 1 ) );
      EXAM_CHECK( a[i]->wait_msg( std::tr2::milliseconds(n * 200), k ) );
    }

    for ( int i = 0; i < n; ++i ) {
      delete a[i];
    }
  }
  catch ( const std::runtime_error& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( std::exception& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( ... ) {
    EXAM_ERROR( "unknown exception" );
  }

  for ( int i = 0; i < n; ++i ) {
    unlink( (std::string( "/tmp/janus." ) + std::string(names[i]) ).c_str() );
  }

  return EXAM_RESULT;
}


} // namespace janus

// -*- C++ -*- Time-stamp: <10/01/21 18:03:13 ptr>

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

namespace janus {

using namespace std;

#define EV_FREE      0x9000

class VTM_one_group_recover :
    public basic_vs
{
  public:
    VTM_one_group_recover();
    VTM_one_group_recover( const stem::addr_type& id );
    ~VTM_one_group_recover();

    template <class Duration>
    bool wait( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );

        return cnd.timed_wait( lk, rel_time, status );
      }

    template <class Duration>
    bool wait_view( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx_view );

        return cnd_view.timed_wait( lk, rel_time, status_view );
      }

    vtime& vt()
      { return basic_vs::self; }

    virtual xmt::uuid_type vs_pub_recover();
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    virtual void vs_pub_view_update();
    virtual void vs_event_origin( const janus::vtime&, const stem::Event& );
    virtual void vs_event_derivative( const vtime&, const stem::Event& );
    virtual void vs_pub_flush();

    std::string mess;

    void reset()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); pass = false; }
    void reset_view()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx_view ); pass_view = false; }

  private:
    void message( const stem::Event& );

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    bool pass;
    std::tr2::mutex mtx_view;
    std::tr2::condition_variable cnd_view;
    bool pass_view;

    struct _status
    {
        _status( VTM_one_group_recover& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_recover& me;
    } status;

    struct _status_view
    {
        _status_view( VTM_one_group_recover& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_recover& me;
    } status_view;

    std::fstream history;

    DECLARE_RESPONSE_TABLE( VTM_one_group_recover, janus::basic_vs );
};

VTM_one_group_recover::VTM_one_group_recover() :
    basic_vs(),
    pass( false ),
    pass_view( false ),
    status( *this ),
    status_view( *this )
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
    pass( false ),
    pass_view( false ),
    status( *this ),
    status_view( *this )
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

xmt::uuid_type VTM_one_group_recover::vs_pub_recover()
{
  vtime _vt;
  stem::Event ev;
  stem::code_type c;
  uint32_t f;
  xmt::uuid_type flush_id = xmt::nil_uuid;

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
        // _vt.clear();
        // _vt.unpack( history );

        stem::__pack_base::__unpack( history, c );
        ev.code( c );
        stem::__pack_base::__unpack( history, f );
        ev.resetf( f );
        stem::__pack_base::__unpack( history, ev.value() );

        if ( !history.fail() ) {
          if ( history.tellg() <= last_flush_off ) {
            this->replay( _vt, ev );
            if ( ev.code() == basic_vs::VS_FLUSH_VIEW ) {
              stem::Event_base<xmt::uuid_type> fev;
              fev.unpack( ev );              
              flush_id = fev.value();
            }
          } else {
            // keep in mind, that last_flush_off may be 0 here
            // i.e. no flush in history
            history.seekp( max( last_flush_off, static_cast<uint64_t>(sizeof(last_flush_off)) ), ios_base::beg );
            // replay from flush_id ...
            // cerr << HERE << ' ' << flush_id << endl;
            break;
          }
        }
      }
      if ( history.fail() ) {
        history.clear();
        // keep in mind, that last_flush_off may be 0 here
        // i.e. no history yet
        history.seekp( max( last_flush_off, static_cast<uint64_t>(sizeof(last_flush_off)) ), ios_base::beg );
        // replay from flush_id ...
        // cerr << HERE << ' ' << flush_id << endl;
      }
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
    // _vt.clear();
    // _vt.unpack( history );

    stem::__pack_base::__unpack( history, c );
    ev.code( c );
    stem::__pack_base::__unpack( history, f );
    ev.resetf( f );
    stem::__pack_base::__unpack( history, ev.value() );

    if ( !history.fail() ) {
      if ( !ref_point_found ) {
        if ( ev.code() == basic_vs::VS_FLUSH_VIEW ) {
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
        if ( ev.code() == basic_vs::VS_FLUSH_VIEW ) {
          ev.code( basic_vs::VS_FLUSH_VIEW_JOIN );
        }
        Forward( ev ); // src was set above
      }
    }
  }

  history.clear();
  // history.seekp( 0, ios_base::end );
}

void VTM_one_group_recover::vs_pub_view_update()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx_view );
  pass_view = true;
  cnd_view.notify_one();
}

void VTM_one_group_recover::vs_event_origin( const vtime& _vt, const stem::Event& ev )
{
  // _vt.pack( history );
  stem::__pack_base::__pack( history, ev.code() );
  stem::__pack_base::__pack( history, ev.flags() );
  stem::__pack_base::__pack( history, ev.value() );
}

void VTM_one_group_recover::vs_event_derivative( const vtime& _vt, const stem::Event& ev )
{
  // _vt.pack( history );
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

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

bool VTM_one_group_recover::_status::operator()() const
{
  return me.pass;
}

bool VTM_one_group_recover::_status_view::operator()() const
{
  return me.pass_view;
}

void VTM_one_group_recover::message( const stem::Event& ev )
{
  mess = ev.value();

  if ( (ev.flags() & stem::__Event_Base::vs) == 0 ) {
    vs_aux( ev );
  } else if ( (ev.flags() & stem::__Event_Base::vs_join) != 0 ) {
    // This is event come during recovery procedure:
    stem::Event xev;
    ev.pack( xev );
    VTM_one_group_recover::vs_event_derivative( vtime(), xev );
  }

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( VTM_one_group_recover )
  EV_EDS( ST_NULL, EV_FREE, message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VT_one_group_recover)
{
  stem::addr_type a1_stored;
  stem::addr_type a2_stored;
  stem::addr_type a3_stored;

  try {
    VTM_one_group_recover a1;
    VTM_one_group_recover a2;

    a1_stored = a1.self_id();

    // first, but join required for vs_pub_recover:
    a1.vs_join( stem::badaddr );

    a2_stored = a2.self_id();
  
    a2.vs_join( a1.self_id() );

    EXAM_CHECK( a2.wait_view( std::tr2::milliseconds(500) ) );

    try {
      VTM_one_group_recover a3;

      a3_stored = a3.self_id();

      a1.reset();
      a2.reset();
      a1.reset_view();
      a2.reset_view();

      a3.vs_join( a1.self_id() );

      EXAM_CHECK( a2.wait_view( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a3.wait_view( std::tr2::milliseconds(500) ) );
    
      EXAM_CHECK( a1.vt()[a1.self_id()] == 3 );
      EXAM_CHECK( a1.vt()[a2.self_id()] == 0 );
      EXAM_CHECK( a1.vt()[a3.self_id()] == 0 );
      
      EXAM_CHECK( a2.vt()[a1.self_id()] == 3 );
      EXAM_CHECK( a2.vt()[a2.self_id()] == 0 );
      EXAM_CHECK( a2.vt()[a3.self_id()] == 0 );
      
      EXAM_CHECK( a3.vt()[a1.self_id()] == 3 );
      EXAM_CHECK( a3.vt()[a2.self_id()] == 0 );
      EXAM_CHECK( a3.vt()[a3.self_id()] == 0 );    

      a1.reset();
      a2.reset();
      a3.reset();

      stem::Event ev( EV_FREE );
      ev.value() = "message";
      ev.dest( a1.self_id() );

      a1.Send( ev );

      EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a1.mess == "message" );
      EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a2.mess == "message" );
      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a3.mess == "message" );

      a1.reset();
      a2.reset();
      a3.reset();

      a1.vs_send_flush();

      EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

      a1.reset_view();
      a2.reset_view();
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

    EXAM_CHECK( a1.wait_view( std::tr2::milliseconds(300) ) || a2.wait_view( std::tr2::milliseconds(300) ) );

    a1.reset_view();
    a2.reset_view();

    {
      a1.reset();
      a2.reset();

      stem::Event ev( EV_FREE );
      ev.value() = "extra message";
      ev.dest( a1.self_id() );

      a1.Send( ev );

      EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a1.mess == "extra message" );
      EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a2.mess == "extra message" );
      
      EXAM_CHECK( a1.vt()[a1.self_id()] >= 7 );
      EXAM_CHECK( a1.vt()[a2.self_id()] >= 0 );

      EXAM_CHECK( a2.vt()[a1.self_id()] >= 7 );
      EXAM_CHECK( a2.vt()[a2.self_id()] >= 0 );
      
      a1.reset();
      a2.reset();
    }

    try {

      VTM_one_group_recover a3( a3_stored );

      a3.vs_join( a2.self_id() );

      EXAM_CHECK( a1.wait_view( std::tr2::milliseconds(500) ) );
      // EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
      // a3 not only join, but replay too...
      EXAM_CHECK( a3.wait_view( std::tr2::milliseconds(500) ) );
      // so I can't just check a3's vtime...
      
      EXAM_CHECK( a1.vt()[a1.self_id()] >= 7 );
      EXAM_CHECK( a1.vt()[a2.self_id()] >= 2 );
      EXAM_CHECK( a1.vt()[a3.self_id()] == 0 );

      EXAM_CHECK( a2.vt()[a1.self_id()] >= 7 );
      EXAM_CHECK( a2.vt()[a2.self_id()] >= 2 );
      EXAM_CHECK( a2.vt()[a3.self_id()] == 0 );

      // a3 not only join, but replay too...
      // so we may dalay here...
      // std::tr2::this_thread::sleep( std::tr2::milliseconds(200) );
      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

      EXAM_CHECK( a3.vt()[a1.self_id()] >= 7 );
      EXAM_CHECK( a3.vt()[a2.self_id()] >= 2 );
      EXAM_CHECK( a3.vt()[a3.self_id()] == 0 );

      EXAM_CHECK( a1.mess == "extra message" );
      EXAM_CHECK( a2.mess == "extra message" );
      // EXAM_CHECK( a3.mess == "message" );
      EXAM_CHECK( a3.mess == "extra message" );
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

  unlink( (std::string( "/tmp/janus." ) + std::string(a1_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a2_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a3_stored) ).c_str() );

  return EXAM_RESULT;
}

} // namespace janus

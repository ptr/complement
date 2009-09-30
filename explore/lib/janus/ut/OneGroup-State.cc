// -*- C++ -*- Time-stamp: <09/09/30 16:48:59 ptr>

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

#define EV_FREE      0x9000

class VTM_one_group_advanced_handler :
    public basic_vs
{
  public:
    VTM_one_group_advanced_handler();
    VTM_one_group_advanced_handler( const stem::addr_type& id );
    ~VTM_one_group_advanced_handler();

    template <class Duration>
    bool wait( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );

        return cnd.timed_wait( lk, rel_time, status );
      }

    vtime_matrix_type& vt()
      { return basic_vs::vt; }

    virtual void vs_pub_recover();
    virtual void vs_pub_view_update();
    virtual void vs_event_origin( const janus::vtime&, const stem::Event& );
    virtual void vs_event_derivative( const vtime&, const stem::Event& );

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
        _status( VTM_one_group_advanced_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_advanced_handler& me;
    } status;

    std::fstream history;

    DECLARE_RESPONSE_TABLE( VTM_one_group_advanced_handler, janus::basic_vs );
};

VTM_one_group_advanced_handler::VTM_one_group_advanced_handler() :
    basic_vs(),
    pass( false ),
    status( *this )
{
  string nm( "/tmp/janus." );
  nm += std::string( self_id() );

  history.open( nm.c_str(), ios_base::in | ios_base::out | ios_base::app );

  enable();
}

VTM_one_group_advanced_handler::VTM_one_group_advanced_handler( const stem::addr_type& id ) :
    basic_vs( id ),
    pass( false ),
    status( *this )
{
  string nm( "/tmp/janus." );
  nm += std::string( self_id() );

  history.open( nm.c_str(), ios_base::in | ios_base::out | ios_base::app );

  enable();
}

VTM_one_group_advanced_handler::~VTM_one_group_advanced_handler()
{
  disable();
}

void VTM_one_group_advanced_handler::vs_pub_recover()
{
  vtime _vt;
  stem::Event ev;
  stem::code_type c;
  uint32_t f;

  try {
    // Try to read serialized events and re-play history.
    // Note: don't call round1_start from ctor,
    // because
    //   - it _virtual_ function
    //   - it call 'replay' that call StEM's Dispatch,
    //     and Dispatch is virtual too, and processing
    //     depend upon data from this class.
    history.seekg( 0, ios_base::beg );
    while ( !history.fail() ) {
      _vt.clear();
      _vt.unpack( history );

      stem::__pack_base::__unpack( history, c );
      ev.code( c );
      stem::__pack_base::__unpack( history, f );
      ev.resetf( f );
      stem::__pack_base::__unpack( history, ev.value() );

      if ( !history.fail() ) {
        this->replay( _vt, ev );
      }
    }
    history.clear();
    history.seekp( 0, ios_base::end );
  }
  catch ( const std::runtime_error& err ) {
    history.clear();
    history.seekp( 0, ios_base::end );
  }
}

void VTM_one_group_advanced_handler::vs_pub_view_update()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

void VTM_one_group_advanced_handler::vs_event_origin( const vtime& _vt, const stem::Event& ev )
{
  _vt.pack( history );
  stem::__pack_base::__pack( history, ev.code() );
  stem::__pack_base::__pack( history, ev.flags() );
  stem::__pack_base::__pack( history, ev.value() );
}

void VTM_one_group_advanced_handler::vs_event_derivative( const vtime& _vt, const stem::Event& ev )
{
  _vt.pack( history );
  stem::__pack_base::__pack( history, ev.code() );
  stem::__pack_base::__pack( history, ev.flags() );
  stem::__pack_base::__pack( history, ev.value() );
}

bool VTM_one_group_advanced_handler::_status::operator()() const
{
  return me.pass;
}

void VTM_one_group_advanced_handler::message( const stem::Event& ev )
{
  mess = ev.value();

  if ( (ev.flags() & stem::__Event_Base::vs) == 0 ) {
    vs( ev );
  }

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( VTM_one_group_advanced_handler )
  EV_EDS( ST_NULL, EV_FREE, message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VT_one_group_replay)
{
  stem::addr_type a1_stored;
  stem::addr_type a2_stored;
  stem::addr_type a3_stored;

  try {
    VTM_one_group_advanced_handler a1;
    VTM_one_group_advanced_handler a2;

    a1_stored = a1.self_id();
    a2_stored = a2.self_id();
  
    a2.vs_join( a1.self_id() );

    EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

    try {
      VTM_one_group_advanced_handler a3;

      a3_stored = a3.self_id();
      a3.vs_join( a2.self_id() );

      EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

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

      a1.reset();
      a2.reset();
      a3.reset();

      stem::Event ev( EV_FREE );
      ev.value() = "message";
      ev.dest( a1.self_id() );

      a1.Send( ev ); // simulate outer event

      EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a1.mess == "message" );
      EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a2.mess == "message" );
      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a3.mess == "message" );
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

    try {
      a1.reset();
      a2.reset();

      VTM_one_group_advanced_handler a3( /* super_spirit.back() */ a3_stored );

      a3.vs_join( a2.self_id() );

      EXAM_CHECK( a1.wait( std::tr2::milliseconds(500) ) );
      // a3 not only join, but replay too...
      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );
      // so I can't just check a3's vtime...

      // EXAM_CHECK( a3.vt()[a3.self_id()][a1.self_id()] == 0 );
      // EXAM_CHECK( a3.vt()[a3.self_id()][a2.self_id()] == 4 );

      EXAM_CHECK( a3.mess == "message" );
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

int EXAM_IMPL(vtime_operations::VT_one_group_late_replay)
{
  stem::addr_type a1_stored;
  stem::addr_type a2_stored;
  stem::addr_type a3_stored;

  try {
    VTM_one_group_advanced_handler a1;
    VTM_one_group_advanced_handler a2;

    a1_stored = a1.self_id();
    a2_stored = a2.self_id();
  
    a2.vs_join( a1.self_id() );

    EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

    try {
      VTM_one_group_advanced_handler a3;

      a3_stored = a3.self_id();

      a1.reset();
      a2.reset();

      a3.vs_join( a1.self_id() );


      EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

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
    }

    try {
      VTM_one_group_advanced_handler a3( a3_stored );

      a3.vs_join( a2.self_id() );

      EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
      // a3 not only join, but replay too...
      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );
      // so I can't just check a3's vtime...

      // EXAM_CHECK( a3.vt()[a3.self_id()][a1.self_id()] == 1 );
      // EXAM_CHECK( a3.vt()[a3.self_id()][a2.self_id()] == 0 );
      EXAM_CHECK( a1.vt()[a1.self_id()][a1.self_id()] == 5 );
      EXAM_CHECK( a1.vt()[a1.self_id()][a2.self_id()] == 0 );
      EXAM_CHECK( a1.vt()[a1.self_id()][a3.self_id()] == 0 );
      EXAM_CHECK( a2.vt()[a2.self_id()][a1.self_id()] == 5 );
      EXAM_CHECK( a2.vt()[a2.self_id()][a2.self_id()] == 0 );
      EXAM_CHECK( a2.vt()[a2.self_id()][a3.self_id()] == 0 );

      EXAM_CHECK( a1.mess == "extra message" );
      EXAM_CHECK( a2.mess == "extra message" );
      EXAM_CHECK( a3.mess == "message" );
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

// -*- C++ -*- Time-stamp: <09/09/18 13:05:00 ptr>

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

    template <class Duration>
    bool wait( const Duration& rel_time )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );

        return cnd.timed_wait( lk, rel_time, status );
      }

    vtime_matrix_type& vt()
      { return basic_vs::vt; }

    virtual void round1_start();
    virtual void round2_pass();
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
    status( *this ),
    history( (std::string( "/tmp/janus." ) + std::string( self_id() ) ).c_str(), ios_base::in | ios_base::out | ios_base::app )
{
  enable();
}

VTM_one_group_advanced_handler::VTM_one_group_advanced_handler( const stem::addr_type& id ) :
    basic_vs( id ),
    pass( false ),
    status( *this ),
    history( (std::string( "/tmp/janus." ) + std::string( self_id() ) ).c_str(), ios_base::in | ios_base::out | ios_base::app )
{
  enable();
}

void VTM_one_group_advanced_handler::round1_start()
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

void VTM_one_group_advanced_handler::round2_pass()
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

  EXAM_CHECK_ASYNC( (ev.flags() & stem::__Event_Base::vs) != 0 );

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  pass = true;
  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( VTM_one_group_advanced_handler )
  EV_EDS( ST_NULL, EV_FREE, message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VT_one_group_replay)
{
  list<stem::addr_type> super_spirit;

  try {
    VTM_one_group_advanced_handler a1;
    VTM_one_group_advanced_handler a2;

    a1.VTjoin( super_spirit.begin(), super_spirit.end() );

    super_spirit.push_back( a1.self_id() );
  
    a2.VTjoin( super_spirit.begin(), super_spirit.end() );

    super_spirit.push_back( a2.self_id() );

    EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );

    try {
      VTM_one_group_advanced_handler a3;

      a3.VTjoin( super_spirit.begin(), super_spirit.end() );

      super_spirit.push_back( a3.self_id() );

      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

      a1.reset();
      a2.reset();
      a3.reset();

      stem::Event ev( EV_FREE );
      ev.value() = "message";

      a1.vs( ev );

      EXAM_CHECK( a2.wait( std::tr2::milliseconds(500) ) );
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
      VTM_one_group_advanced_handler a3( super_spirit.back() );

      a3.VTjoin( super_spirit.begin(), super_spirit.end() );

      EXAM_CHECK( a3.wait( std::tr2::milliseconds(500) ) );

      EXAM_CHECK( a3.vt()[a3.self_id()][a1.self_id()] == 1 );
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

  for ( list<stem::addr_type>::const_iterator i = super_spirit.begin(); i != super_spirit.end(); ++i ) {
    unlink( (std::string( "/tmp/janus." ) + std::string(*i) ).c_str() );
  }

  return EXAM_RESULT;
}

} // namespace janus

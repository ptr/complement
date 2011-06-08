// -*- C++ -*- Time-stamp: <2011-06-08 16:50:09 ptr>

/*
 * Copyright (c) 1995-1999, 2002-2003, 2005-2010
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <config/feature.h>
#include "stem/EventHandler.h"
#include "stem/EvManager.h"
#include "stem/Names.h"
#include <mt/mutex>
#include <mt/thread>
#include <mt/uid.h>
#include <exam/defs.h>

#include <unistd.h>

namespace stem {

using namespace std::tr2;

char *Init_buf[128];
static domain_type _domain = xmt::uid();
static EvManager _mgr;
Names     *EventHandler::_ns = 0;
mutex _def_lock;
addr_type _default_addr = xmt::nil_uuid;

static int _rcount = 0;
static mutex _mf_lock;

void EventHandler::Init::__at_fork_prepare()
{
  _mf_lock.lock();
  if ( _rcount != 0 ) {
    _mgr.stop_queue();
  }
}

void EventHandler::Init::__at_fork_child()
{
  _domain = xmt::uid();
  if ( _rcount != 0 ) {
    _mgr.start_queue();
  }
  _mf_lock.unlock();
}

void EventHandler::Init::__at_fork_parent()
{
  if ( _rcount != 0 ) {
    _mgr.start_queue();
  }
  _mf_lock.unlock();
}

static void sq()
{
  _mgr.stop_queue();
}

void EventHandler::Init::_guard( int direction )
{
  static recursive_mutex _init_lock;
  static bool at_f = false;

  lock_guard<recursive_mutex> lk(_init_lock);

  if ( direction ) {
    if ( _rcount++ == 0 ) {
      if ( !at_f ) {
        // cerr << HERE << endl;
        atexit( &sq );
        pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );
        at_f = true;

        _mgr.start_queue();
        EventHandler::_ns = new Names( "ns" );
        EventHandler::_ns->enable();
      } else if ( EventHandler::_ns == 0 ) {
        EventHandler::_ns = new Names( "ns" );
        EventHandler::_ns->enable();
      }
    }
  } else {
    --_rcount;
    // cerr << HERE << ' ' << _rcount << endl;
    if ( _rcount == 1 ) {
      EventHandler::_ns->disable();
      delete EventHandler::_ns;
      EventHandler::_ns = 0;
      // cerr << HERE << endl;
    } else if ( _rcount == 0 ) {
      // _mgr.stop_queue();
      // cerr << HERE << endl;
    }
  }
}

EventHandler::Init::Init()
{ _guard( 1 ); }

EventHandler::Init::~Init()
{ _guard( 0 ); }

EvManager& EventHandler::manager()
{ return _mgr; }

bool EventHandler::is_avail( const addr_type& id ) const
{
  return _mgr.is_avail( id );
}

void EventHandler::Send( const Event& e ) const
{
  e.src( _id );
  _mgr.push( e );
}

void EventHandler::Forward( const Event& e ) const
{
  _mgr.push( e );
}

void EventHandler::sync_call( const Event& e )
{
  _mgr.push( e );
}

void EventHandler::PushState( state_type state )
{
  RemoveState( state );
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  theHistory.push_front( state );
}

state_type EventHandler::State() const
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  if ( theHistory.empty() ) {
    return ST_TERMINAL;
  }
  state_type top = theHistory.front();
  return top;
}

void EventHandler::PushTState( state_type state )
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  theHistory.push_front( ST_TERMINAL );
  theHistory.push_front( state );
}

void EventHandler::PopState()
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  if ( theHistory.empty() ) {
    return;
  }
  theHistory.pop_front();
  while ( theHistory.front() == ST_TERMINAL ) {
    theHistory.pop_front();
  }
}

void EventHandler::PopState( state_type state )
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( theHistory.begin(), ++hst_i );
  }
}

void EventHandler::RemoveState( state_type state )
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  if ( theHistory.empty() ) {
    return;
  }
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( hst_i );
  }
}

bool EventHandler::isState( state_type state ) const
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  const HistoryContainer& hst = theHistory;
  const_h_iterator hst_i = __find( state );
  if ( hst_i != hst.end() && *hst_i != ST_TERMINAL ) {
    return true;
  }
  return false;
}

h_iterator EventHandler::__find( state_type state )
{
  if ( theHistory.empty() ) {
    return theHistory.end();
  }
  h_iterator hst_i = theHistory.begin();

  while ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL
	  && *hst_i != state ) {
    ++hst_i;
  }
  return hst_i;
}

const_h_iterator EventHandler::__find( state_type state ) const
{
  if ( theHistory.empty() ) {
    return theHistory.end();
  }
  const_h_iterator hst_i = theHistory.begin();

  while ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL
	  && *hst_i != state ) {
    ++hst_i;
  }
  return hst_i;
}

EventHandler::EventHandler() :
    _nice( 0 ),
    _id( xmt::uid() )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
}

EventHandler::EventHandler( const char* info ) :
    _nice( 0 ),
    _id( xmt::uid() )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _mgr.annotate( _id, info );
}

EventHandler::EventHandler( const addr_type& id, int nice ) :
    _nice( nice ),
    _id( id )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
}

EventHandler::EventHandler( const addr_type& id, const char* info ) :
    _nice( 0 ),
    _id( id )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _mgr.annotate( _id, info );
}

EventHandler::~EventHandler()
{
  EventHandler::solitary();
  Init* tmp = reinterpret_cast<Init*>(Init_buf);
  tmp->~Init();
}

void EventHandler::TraceStack( ostream& out ) const
{
  std::tr2::lock_guard<std::tr2::recursive_mutex> lk( _theHistory_lock );
  const HistoryContainer& hst = theHistory;
  HistoryContainer::const_iterator hst_i = hst.begin();
  while ( hst_i != hst.end() ) {
    out << "[" << *hst_i++ << "]";
  }
  out << endl;
}

addr_type EventHandler::ns()
{ return _ns->self_id(); }

const domain_type& EventHandler::domain()
{ return _domain; }

addr_type EventHandler::set_default() const
{
  lock_guard<mutex> lk( _def_lock );
  addr_type tmp = _default_addr;
  _default_addr = _id;

  return tmp;
}

addr_type EventHandler::get_default()
{
  lock_guard<mutex> lk( _def_lock );
  addr_type tmp = _default_addr;

  return tmp;
}

void EventHandler::solitary()
{
  _mgr.Unsubscribe( _id, this );
  {
    std::tr2::lock_guard<std::tr2::recursive_mutex> hlk( _theHistory_lock );
    _id = badaddr;
  }
  theHistory.clear();
}

void EventHandler::enable()
{
  _mgr.Subscribe( _id, this, _nice );
}

void EventHandler::disable()
{
  _mgr.Unsubscribe( _id, this );
  {
    std::tr2::lock_guard<std::tr2::recursive_mutex> hlk( _theHistory_lock );
    _id = badaddr;
  }
}

int EventHandler::flags() const
{
  return 0;
}

} // namespace stem

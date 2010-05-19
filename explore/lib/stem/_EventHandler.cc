// -*- C++ -*- Time-stamp: <10/05/19 17:02:49 ptr>

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

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include "stem/EventHandler.h"
#include "stem/EvManager.h"
#include "stem/Names.h"
#include <mt/mutex>
#include <mt/thread>
#include <mt/uid.h>

#include <unistd.h>

namespace stem {

using namespace std::tr2;

char *Init_buf[128];
EvManager *EventHandler::_mgr = 0;
Names     *EventHandler::_ns = 0;
mutex _def_lock;
addr_type _default_addr = xmt::nil_uuid;

static int *_rcount = 0;
#if 1 // depends where fork happens: in the EvManager loop (stack) or not.
void EventHandler::Init::__at_fork_prepare()
{
}

void EventHandler::Init::__at_fork_child()
{
  if ( *_rcount != 0 ) {
    EventHandler::_mgr->~EvManager();
    EventHandler::_mgr = new( EventHandler::_mgr ) EvManager();
  }
}

void EventHandler::Init::__at_fork_parent()
{
}
#endif

void EventHandler::Init::_guard( int direction )
{
  static recursive_mutex _init_lock;

  lock_guard<recursive_mutex> lk(_init_lock);
  static int _count = 0;

  if ( direction ) {
    if ( _count++ == 0 ) {
#ifdef _PTHREADS
      _rcount = &_count;
      pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );
#endif
      EventHandler::_mgr = new EvManager();
      EventHandler::_ns = new Names( "ns" );
      EventHandler::_ns->enable();
    }
  } else {
    --_count;
    if ( _count == 1 ) {
      EventHandler::_ns->disable();
      delete EventHandler::_ns;
      EventHandler::_ns = 0;
    } else if ( _count == 0 ) {
      delete EventHandler::_mgr;
      EventHandler::_mgr = 0;
    }
  }
}

EventHandler::Init::Init()
{ _guard( 1 ); }

EventHandler::Init::~Init()
{ _guard( 0 ); }

__FIT_DECLSPEC
bool EventHandler::is_avail( const addr_type& id ) const
{
  return _mgr->is_avail( id );
}

__FIT_DECLSPEC
void EventHandler::Send( const Event& e ) const
{
  e.src( _ids.front() );
  _mgr->push( e );
}

__FIT_DECLSPEC
void EventHandler::Forward( const Event& e ) const
{
  _mgr->push( e );
}

__FIT_DECLSPEC
void EventHandler::sync_call( const Event& e )
{
  _mgr->sync_call( *this, e );
}

__FIT_DECLSPEC
void EventHandler::PushState( state_type state )
{
  RemoveState( state );
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  theHistory.push_front( state );
}

__FIT_DECLSPEC
state_type EventHandler::State() const
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  if ( theHistory.empty() ) {
    return ST_TERMINAL;
  }
  state_type top = theHistory.front();
  return top;
}

__FIT_DECLSPEC
void EventHandler::PushTState( state_type state )
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  theHistory.push_front( ST_TERMINAL );
  theHistory.push_front( state );
}

__FIT_DECLSPEC
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

__FIT_DECLSPEC
void EventHandler::PopState( state_type state )
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( theHistory.begin(), ++hst_i );
  }
}

__FIT_DECLSPEC
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

__FIT_DECLSPEC
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

__FIT_DECLSPEC
EventHandler::EventHandler() :
    _nice( 0 )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _ids.push_back( xmt::uid() );
}

__FIT_DECLSPEC
EventHandler::EventHandler( const char* info ) :
    _nice( 0 )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _ids.push_back( xmt::uid() );
  _mgr->annotate( _ids.back(), info );
}

__FIT_DECLSPEC
EventHandler::EventHandler( const addr_type& id, int nice ) :
    _nice( nice )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _ids.push_back( id );
}

__FIT_DECLSPEC
EventHandler::EventHandler( const addr_type& id, const char* info ) :
    _nice( 0 )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _ids.push_back( id );
  _mgr->annotate( id, info );
}

__FIT_DECLSPEC
EventHandler::~EventHandler()
{
  EventHandler::solitary();
  ((Init *)Init_buf)->~Init();
}

__FIT_DECLSPEC
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

addr_type EventHandler::set_default() const
{
  lock_guard<mutex> lk( _def_lock );
  addr_type tmp = _default_addr;
  _default_addr = _ids.front();

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
  lock_guard<recursive_mutex> lk( _theHistory_lock );

  for ( addr_container_type::iterator i = _ids.begin(); i != _ids.end(); ) {
    _mgr->Unsubscribe( *i, this );
    _ids.erase( i++ );
  }

  theHistory.clear();
}

void EventHandler::enable()
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );

  for ( addr_container_type::iterator i = _ids.begin(); i != _ids.end(); ++i ) {
    _mgr->Subscribe( *i, this, _nice );
  }
}

void EventHandler::disable()
{
  lock_guard<recursive_mutex> lk( _theHistory_lock );

  for ( addr_container_type::iterator i = _ids.begin(); i != _ids.end(); ++i ) {
    _mgr->Unsubscribe( *i, this );
  }
}

int EventHandler::flags() const
{
  return 0;
}

} // namespace stem

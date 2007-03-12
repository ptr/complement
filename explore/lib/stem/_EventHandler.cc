// -*- C++ -*- Time-stamp: <07/03/12 19:36:45 ptr>

/*
 * Copyright (c) 1995-1999, 2002, 2003, 2005, 2006
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
#include "mt/xmt.h"

#include <unistd.h>

namespace stem {

char *Init_buf[128];
EvManager *EventHandler::_mgr = 0;
Names     *EventHandler::_ns = 0;

static int *_rcount = 0;
#if 1 // depends where fork happens: in the EvManager loop (stack) or not.
void EventHandler::Init::__at_fork_prepare()
{
}

void EventHandler::Init::__at_fork_child()
{
  if ( *_rcount != 0 ) {
    delete EventHandler::_mgr;
    EventHandler::_mgr = new( EventHandler::_mgr ) EvManager();
  }
}

void EventHandler::Init::__at_fork_parent()
{
}
#endif

void EventHandler::Init::_guard( int direction )
{
  static xmt::MutexRS _init_lock;
  static int _count = 0;

  if ( direction ) {
    if ( _count++ == 0 ) {
#ifdef _PTHREADS
      _rcount = &_count;
      pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );
#endif
      EventHandler::_mgr = new EvManager();
      EventHandler::_ns = new Names( ns_addr, "ns" );
    }
  } else {
    --_count;
    if ( _count == 1 ) {
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
const string EventHandler::who_is( addr_type k ) const
{
  return _mgr->who_is( k );
}

__FIT_DECLSPEC
bool EventHandler::is_avail( addr_type id ) const
{
  return _mgr->is_avail( id );
}

__FIT_DECLSPEC
void EventHandler::Send( const Event& e )
{
  e.src( _id );
  _mgr->push( e );
}

__FIT_DECLSPEC
void EventHandler::Send( const EventVoid& e )
{
  e.src( _id );
  _mgr->push( stem::Event_convert<void>()( e ) );
}

__FIT_DECLSPEC
void EventHandler::Forward( const Event& e )
{
  _mgr->push( e );
}

__FIT_DECLSPEC
void EventHandler::Forward( const EventVoid& e )
{
  _mgr->push( stem::Event_convert<void>()( e ) );
}

__FIT_DECLSPEC
void EventHandler::PushState( state_type state )
{
  RemoveState( state );
  MT_REENTRANT_SDS( _theHistory_lock, _x1 );
  theHistory.push_front( state );
}

__FIT_DECLSPEC
state_type EventHandler::State() const
{
  MT_REENTRANT_SDS( _theHistory_lock, _x1 );
  return theHistory.front();
}

__FIT_DECLSPEC
void EventHandler::PushTState( state_type state )
{
  MT_REENTRANT_SDS( _theHistory_lock, _x1 );
  theHistory.push_front( ST_TERMINAL );
  theHistory.push_front( state );
}

__FIT_DECLSPEC
void EventHandler::PopState()
{
  MT_REENTRANT_SDS( _theHistory_lock, _x1 );
  theHistory.pop_front();
  while ( theHistory.front() == ST_TERMINAL ) {
    theHistory.pop_front();
  }
}

__FIT_DECLSPEC
void EventHandler::PopState( state_type state )
{
  MT_REENTRANT_SDS( _theHistory_lock, _x1 );
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( theHistory.begin(), ++hst_i );
  }
}

__FIT_DECLSPEC
void EventHandler::RemoveState( state_type state )
{
  MT_REENTRANT_SDS( _theHistory_lock, _x1 );
  h_iterator hst_i = __find( state );
  if ( hst_i != theHistory.end() && *hst_i != ST_TERMINAL ) {
    theHistory.erase( hst_i );
  }
}

__FIT_DECLSPEC
bool EventHandler::isState( state_type state ) const
{
  MT_REENTRANT_SDS( _theHistory_lock, _x1 );
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
EventHandler::EventHandler()
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _id = _mgr->Subscribe( this );
}

__FIT_DECLSPEC
EventHandler::EventHandler( const char *info )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _id = _mgr->Subscribe( this, info );
}

__FIT_DECLSPEC
EventHandler::EventHandler( addr_type id, const char *info )
{
  new( Init_buf ) Init();
  theHistory.push_front( ST_NULL );  // State( ST_NULL );
  _id = _mgr->SubscribeID( id, this, info );
  // _STLP_ASSERT( _id != -1 ); // already registered, or id has Event::extbit
//  if ( _id == -1 ) {
//    _mgr->Subscribe( this, "" );
//  }
}

__FIT_DECLSPEC
EventHandler::~EventHandler()
{
   _mgr->Unsubscribe( _id );
  ((Init *)Init_buf)->~Init();
}

__FIT_DECLSPEC
void EventHandler::TraceStack( ostream& out ) const
{
  MT_REENTRANT_RS( _theHistory_lock, _x1 );
  const HistoryContainer& hst = theHistory;
  HistoryContainer::const_iterator hst_i = hst.begin();
  while ( hst_i != hst.end() ) {
    out << "[" << *hst_i++ << "]";
  }
  out << endl;
}

__FIT_DECLSPEC
gaddr_type EventHandler::self_glid() const
{
  return gaddr_type(xmt::hostid(), xmt::getpid(), _id );
}

} // namespace stem

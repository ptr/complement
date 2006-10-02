// -*- C++ -*- Time-stamp: <06/09/30 09:44:46 ptr>

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

#if defined(__TRACE) || defined(__WARN)

// DIAG_DEFINE_GROUP(EventHandler,0,4);
// DIAG_DEFINE_GROUP(EventDispatch,0,3);

#endif // __TRACE || __WARN

namespace stem {

char *Init_buf[32];
EvManager *EventHandler::_mgr = 0;
stem::Names *_ns = 0;
const char *_ns_name = "ns";

int EventHandler::Init::_count = 0;
xmt::MutexRS _init_lock;

EventHandler::Init::Init()
{
  MT_REENTRANT_RS( _init_lock, _x );
  if ( _count++ == 0 ) {
    EventHandler::_mgr = new EvManager();
    stem::_ns = new Names( ns_addr, _ns_name );
  }
}

EventHandler::Init::~Init()
{
  MT_REENTRANT_RS( _init_lock, _x );
  --_count;
  if ( _count == 1 ) {
    delete stem::_ns;
  } else if ( _count == 0 ) {
    delete EventHandler::_mgr;
  }
}

__FIT_DECLSPEC
const string& EventHandler::who_is( addr_type k ) const
{
  return _mgr->who_is( k );
}

__FIT_DECLSPEC
bool EventHandler::is_avail( addr_type id ) const
{
  return _mgr->is_avail( id );
}

__FIT_DECLSPEC
key_type EventHandler::sid( addr_type k ) const
{
  return _mgr->sid( k );
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

} // namespace stem

// -*- C++ -*- Time-stamp: <06/12/15 03:12:28 ptr>

/*
 * Copyright (c) 1998, 2002, 2003, 2005, 2006
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
#pragma warning( disable : 4800 )
#endif

#include <config/feature.h>
#include "stem/Cron.h"
#include "stem/EvManager.h"
#include "stem/EDSEv.h"

#include <cmath>

#define CRON_ST_STARTED   0x10
#define CRON_ST_SUSPENDED 0x11

namespace stem {

__FIT_DECLSPEC Cron::Cron() :
    EventHandler()
{
}

__FIT_DECLSPEC Cron::Cron( const char *info ) :
    EventHandler( info )
{
}

__FIT_DECLSPEC Cron::Cron( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

__FIT_DECLSPEC Cron::~Cron()
{
  if ( isState( CRON_ST_STARTED ) ) {
    Stop();
  }
}

void __FIT_DECLSPEC Cron::AddFirst( const Event_base<CronEntry>& entry )
{
  Add( entry );
  Start();
}

void __FIT_DECLSPEC Cron::Add( const Event_base<CronEntry>& entry )
{
  const CronEntry& ne = entry.value();
  __CronEntry en;

  en.code = ne.code;
  en.addr = entry.src();
  en.start = ne.start;
  en.n = ne.n;
  en.arg = ne.arg;
  if ( en.n == 1 ) {
    en.period = 0;
  } else {
    en.period = ne.period;
    if ( en.n == 0 || en.period.tv_nsec >= 1000000000 ||
         (en.period.tv_sec == 0 && en.period.tv_nsec == 0) ) 
      return;
  }

  time_t current;
  if ( en.start.tv_sec < time( &current ) ) {
    en.start.tv_sec = current;
  }

  en.expired = en.start;
  en.count = 0;

  MT_REENTRANT( _M_l, _x1 );
  _M_c.push( en );
  if ( isState( CRON_ST_SUSPENDED ) ) { // alarm if cron loop suspended
    PopState( CRON_ST_SUSPENDED );
    _thr.resume();
  } else if ( _M_c.top() == en ) { // cron has entries, but new entry is on top
    cond.signal();                 // so, we need wait it
  }
}

// Remove cron entry if recipient address and event code match to request
void __FIT_DECLSPEC Cron::Remove( const Event_base<CronEntry>& entry )
{
  MT_REENTRANT( _M_l, _x1 );
  cond.signal(); // in any case, remove I something or not

  const CronEntry& ne = entry.value();
  std::vector<value_type> tmp;

  tmp.reserve( _M_c.size() );

  while ( !_M_c.empty() ) {
    if (  _M_c.top().addr != entry.src() || _M_c.top().code != ne.code ) {
      tmp.push_back( _M_c.top() );
    }
    _M_c.pop();
  }

  while ( !tmp.empty() ) {
    _M_c.push( tmp.back() );
    tmp.pop_back();
  }
}

// Remove cron entry if recipient address and event code match to request
// and arg the same
void __FIT_DECLSPEC Cron::RemoveArg( const Event_base<CronEntry>& entry )
{
  MT_REENTRANT( _M_l, _x1 );
  cond.signal(); // in any case, remove I something or not

  const CronEntry& ne = entry.value();
  std::vector<value_type> tmp;

  tmp.reserve( _M_c.size() );

  while ( !_M_c.empty() ) {
    if (  _M_c.top().addr != entry.src() || _M_c.top().code != ne.code 
          || _M_c.top().arg != ne.arg ) {
      tmp.push_back( _M_c.top() );
    }
    _M_c.pop();
  }

  while ( !tmp.empty() ) {
    _M_c.push( tmp.back() );
    tmp.pop_back();
  }
}

void __FIT_DECLSPEC Cron::Start()
{
  MT_REENTRANT( _M_l, _x1 );
  if ( !_M_c.empty() ) { // start only if Cron queue not empty
    _thr.launch( _loop, this );
  }
}

void __FIT_DECLSPEC Cron::Stop()
{
  RemoveState( CRON_ST_STARTED );
  cond.signal();
  if ( isState( CRON_ST_SUSPENDED ) ) {
    RemoveState( CRON_ST_SUSPENDED );
    _thr.resume();
  }
  _thr.join();
}

void __FIT_DECLSPEC Cron::EmptyStart()
{
  // do nothing
}

void __FIT_DECLSPEC Cron::EmptyStop()
{
  // do nothing
}

xmt::Thread::ret_code Cron::_loop( void *p )
{
  // After creation cron loop (one per every Cron object),
  // this loop should exit in following cases:
  //   -) Cron receive Stop event (if it loop already started)
  //   -) Cron object destroyed
  // If Cron's container empty, this thread suspend, and can be alarmed
  // after Add (Cron entry) event.
  Cron& me = *reinterpret_cast<Cron *>(p);

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  me.PushState( CRON_ST_STARTED );

  timespec abstime;
  while ( me.isState( CRON_ST_STARTED ) ) {
    MT_LOCK( me._M_l );
    if ( me._M_c.empty() ) {
      me.PushState( CRON_ST_SUSPENDED );
      MT_UNLOCK( me._M_l );
      me._thr.suspend();
      continue;
    }
    // At this point _M_c should never be empty!
    abstime = me._M_c.top().expired;
    MT_UNLOCK( me._M_l );
    if ( me.cond.wait_time( &abstime ) > 0 ) { // time expired, otherwise signal or error
      MT_LOCK( me._M_l );

      if ( me._M_c.empty() ) { // event removed while I wait?
        MT_UNLOCK( me._M_l );
        continue;
      }
      __CronEntry en = me._M_c.top(); // get and eject top cron entry
      me._M_c.pop();

      if ( me.is_avail( en.addr ) ) { // check if target abonent exist
        Event_base<unsigned> ev( en.code );
        ev.dest( en.addr );
        ev.value() = en.arg;
        me.Send( Event_convert<unsigned>()( ev ) );
      } else { // do nothing, this lead to removing invalid abonent from Cron
        MT_UNLOCK( me._M_l );
        continue;
      }

      en.expired = en.start;
      en.expired += en.period * ++en.count;

      // if loop infinite, always put Cron entry in stack,
      // otherwise check counter
      if ( en.n == CronEntry::infinite ) {
        en.start = en.expired;
        en.count = 0;
        me._M_c.push( en );
      } else if ( (en.count) < (en.n) ) { // This SC5.0 patch 107312-06 bug
        me._M_c.push( en );
      }
      MT_UNLOCK( me._M_l );
    }
  }

  return rt;
}


DEFINE_RESPONSE_TABLE( Cron )
  EV_Event_base_T_(ST_NULL,EV_EDS_CRON_ADD,AddFirst,CronEntry)
  EV_Event_base_T_(CRON_ST_STARTED,EV_EDS_CRON_ADD,Add,CronEntry)
  EV_Event_base_T_(ST_NULL,EV_EDS_CRON_REMOVE,Remove,CronEntry)
  EV_Event_base_T_(ST_NULL,EV_EDS_CRON_REMOVE_ARG,RemoveArg,CronEntry)
  EV_VOID(ST_NULL,EV_EDS_CRON_START,Start)
  EV_VOID(CRON_ST_STARTED,EV_EDS_CRON_STOP,Stop)
  EV_VOID(CRON_ST_STARTED,EV_EDS_CRON_START,EmptyStart)
  EV_VOID(ST_NULL,EV_EDS_CRON_STOP,EmptyStop)
END_RESPONSE_TABLE

__FIT_DECLSPEC
void CronEntry::pack( std::ostream& s ) const
{
  __pack( s, code );
  __pack( s, start.tv_sec );
  __pack( s, start.tv_nsec );
  __pack( s, period.tv_sec );
  __pack( s, period.tv_nsec );
  __pack( s, n );
  __pack( s, arg );
}

__FIT_DECLSPEC
void CronEntry::net_pack( std::ostream& s ) const
{
  __net_pack( s, code );
  __net_pack( s, start.tv_sec );
  __net_pack( s, start.tv_nsec );
  __net_pack( s, period.tv_sec );
  __net_pack( s, period.tv_nsec );
  __net_pack( s, n );
  __net_pack( s, arg );
}

__FIT_DECLSPEC
void CronEntry::unpack( std::istream& s )
{
  __unpack( s, code );
  __unpack( s, start.tv_sec );
  __unpack( s, start.tv_nsec );
  __unpack( s, period.tv_sec );
  __unpack( s, period.tv_nsec );
  __unpack( s, n );
  __unpack( s, arg );
}

__FIT_DECLSPEC
void CronEntry::net_unpack( std::istream& s )
{
  __net_unpack( s, code );
  __net_unpack( s, start.tv_sec );
  __net_unpack( s, start.tv_nsec );
  __net_unpack( s, period.tv_sec );
  __net_unpack( s, period.tv_nsec );
  __net_unpack( s, n );
  __net_unpack( s, arg );
}

} // namespace stem

// -*- C++ -*- Time-stamp: <01/01/29 13:24:33 ptr>

/*
 * Copyright (c) 1998
 * Petr Ovchenkov
 * 
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 * 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#pragma warning( disable : 4800 )
#endif

#include <config/feature.h>
#include "EDS/Cron.h"
#include "EDS/EvManager.h"
#include "EDS/EDSEv.h"

#include <cmath>

#define CRON_ST_STARTED   0x10
#define CRON_ST_SUSPENDED 0x11

namespace EDS {

__PG_DECLSPEC Cron::Cron() :
    EventHandler()
{
}

__PG_DECLSPEC Cron::Cron( const char *info ) :
    EventHandler( info )
{
}

__PG_DECLSPEC Cron::Cron( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

__PG_DECLSPEC Cron::~Cron()
{
  if ( isState( CRON_ST_STARTED ) ) {
    Stop();
  }
}

void __PG_DECLSPEC Cron::AddFirst( const Event_base<CronEntry>& entry )
{
  Add( entry );
  Start();
}

void __PG_DECLSPEC Cron::Add( const Event_base<CronEntry>& entry )
{
  const CronEntry& ne = entry.value();
  __CronEntry en;

  en.code = ne.code;
  en.addr = entry.src();
  en.start.tv_sec = ne.start.tv_sec;
  en.start.tv_nsec = ne.start.tv_nsec;
  en.n = ne.n;
  en.arg = ne.arg;
  if ( en.n == 1 ) {
    en.period.tv_sec = 0;
    en.period.tv_nsec = 0;
  } else {
    en.period.tv_sec = ne.period.tv_sec;
    en.period.tv_nsec = ne.period.tv_nsec;
    if ( en.n == 0 || en.period.tv_nsec >= 1000000000 ||
         (en.period.tv_sec == 0 && en.period.tv_nsec == 0) ) 
      return;
  }

  time_t current;
  if ( en.start.tv_sec < time( &current ) ) {
    en.start.tv_sec = current;
  }

  en.expired.tv_sec = en.start.tv_sec;
  en.expired.tv_nsec = en.start.tv_nsec;
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
void __PG_DECLSPEC Cron::Remove( const Event_base<CronEntry>& entry )
{
  MT_REENTRANT( _M_l, _x1 );
  cond.signal(); // in any case, remove I something or not

  const CronEntry& ne = entry.value();
  __STD::vector<value_type> tmp;

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
void __PG_DECLSPEC Cron::RemoveArg( const Event_base<CronEntry>& entry )
{
  MT_REENTRANT( _M_l, _x1 );
  cond.signal(); // in any case, remove I something or not

  const CronEntry& ne = entry.value();
  __STD::vector<value_type> tmp;

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

void __PG_DECLSPEC Cron::Start()
{
  MT_REENTRANT( _M_l, _x1 );
  if ( !_M_c.empty() ) { // start only if Cron queue not empty
    _thr.launch( _loop, this );
  }
}

void __PG_DECLSPEC Cron::Stop()
{
  RemoveState( CRON_ST_STARTED );
  cond.signal();
  if ( isState( CRON_ST_SUSPENDED ) ) {
    RemoveState( CRON_ST_SUSPENDED );
    _thr.resume();
  }
  _thr.join();
}

void __PG_DECLSPEC Cron::EmptyStart()
{
  // do nothing
}

void __PG_DECLSPEC Cron::EmptyStop()
{
  // do nothing
}

int Cron::_loop( void *p )
{
  // After creation cron loop (one per every Cron object),
  // this loop should exit in following cases:
  //   -) Cron receive Stop event (if it loop already started)
  //   -) Cron object destroyed
  // If Cron's container empty, this thread suspend, and can be alarmed
  // after Add (Cron entry) event.
  Cron& me = *reinterpret_cast<Cron *>(p);

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

#ifndef __STL_LONG_LONG     
      double _next = en.start.tv_sec + en.start.tv_nsec * 1.0e-9 +
        (en.period.tv_sec + en.period.tv_nsec * 1.0e-9) * ++en.count;
      en.expired.tv_nsec = 1.0e9 * modf( _next, &_next );
      en.expired.tv_sec = _next;
#else
#  ifdef _MSC_VER
      __int64
#  else
      long long 
#  endif
        _d = en.period.tv_nsec;
      _d *= ++en.count;
      _d += en.start.tv_nsec;
      en.expired.tv_sec = en.period.tv_sec;
      en.expired.tv_sec *= en.count;
      en.expired.tv_sec += en.start.tv_sec;
      en.expired.tv_sec += unsigned(_d / 1000000000);
      en.expired.tv_nsec = unsigned(_d % 1000000000);
#endif
      // if loop infinite, always put Cron entry in stack,
      // otherwise check counter
      if ( en.n == CronEntry::infinite ) {
        en.start.tv_sec = en.expired.tv_sec;
        en.start.tv_nsec = en.expired.tv_nsec;        
        en.count = 0;
        me._M_c.push( en );
      } else if ( (en.count) < (en.n) ) { // This SC5.0 patch 107312-06 bug
        me._M_c.push( en );
      }
      MT_UNLOCK( me._M_l );
    }
  }

  return 0;
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

__PG_DECLSPEC
void CronEntry::pack( __STD::ostream& s ) const
{
  __pack( s, code );
  __pack( s, start.tv_sec );
  __pack( s, start.tv_nsec );
  __pack( s, period.tv_sec );
  __pack( s, period.tv_nsec );
  __pack( s, n );
  __pack( s, arg );
}

__PG_DECLSPEC
void CronEntry::net_pack( __STD::ostream& s ) const
{
  __net_pack( s, code );
  __net_pack( s, start.tv_sec );
  __net_pack( s, start.tv_nsec );
  __net_pack( s, period.tv_sec );
  __net_pack( s, period.tv_nsec );
  __net_pack( s, n );
  __net_pack( s, arg );
}

__PG_DECLSPEC
void CronEntry::unpack( __STD::istream& s )
{
  __unpack( s, code );
  __unpack( s, start.tv_sec );
  __unpack( s, start.tv_nsec );
  __unpack( s, period.tv_sec );
  __unpack( s, period.tv_nsec );
  __unpack( s, n );
  __unpack( s, arg );
}

__PG_DECLSPEC
void CronEntry::net_unpack( __STD::istream& s )
{
  __net_unpack( s, code );
  __net_unpack( s, start.tv_sec );
  __net_unpack( s, start.tv_nsec );
  __net_unpack( s, period.tv_sec );
  __net_unpack( s, period.tv_nsec );
  __net_unpack( s, n );
  __net_unpack( s, arg );
}

} // namespace EDS

// -*- C++ -*- Time-stamp: <09/06/08 17:12:25 ptr>

/*
 * Copyright (c) 1998, 2002-2003, 2005-2006, 2008-2009
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

namespace stem {

using namespace std;
using namespace std::tr2;

__FIT_DECLSPEC Cron::Cron() :
    EventHandler(),
    running( *this ),
    ready( *this ),
    _thr( 0 )
{
}

__FIT_DECLSPEC Cron::Cron( const char *info ) :
    EventHandler( info ),
    running( *this ),
    ready( *this ),
    _thr( 0 )
{
}

__FIT_DECLSPEC Cron::Cron( addr_type id, const char *info ) :
    EventHandler( id, info ),
    running( *this ),
    ready( *this ),
    _thr( 0 )
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
    en.period = nanoseconds(0LL);
  } else {
    en.period = ne.period;
    if ( en.n == 0 || en.period.count() == 0LL ) {
      return;
    }
  }

  system_time current = get_system_time();
  if ( en.start < current ) {
    en.start = current;
  }

  en.expired = en.start;
  en.count = 0;

  lock_guard<mutex> _x1( _M_l );
  _M_c.push( en );
  if ( _M_c.top() == en ) { // cron has entries, but new entry is on top
    cond.notify_one();      // so, we need wait it; if only one entry now,
  }                         // then notification also happen (wake-up)
}

// Remove cron entry if recipient address and event code match to request
void __FIT_DECLSPEC Cron::Remove( const Event_base<CronEntry>& entry )
{
  lock_guard<mutex> _x1( _M_l );

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

  cond.notify_one(); // in any case, remove I something or not
}

// Remove cron entry if recipient address and event code match to request
// and arg the same
void __FIT_DECLSPEC Cron::RemoveArg( const Event_base<CronEntry>& entry )
{
  lock_guard<mutex> _x1( _M_l );

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

  cond.notify_one(); // in any case, remove I something or not
}

void __FIT_DECLSPEC Cron::Start()
{
  lock_guard<mutex> _x1( _M_l );

  if ( !_M_c.empty() && (_thr == 0) ) { // start only if Cron queue not empty
    _thr = new thread( _loop, this );
  }
}

void __FIT_DECLSPEC Cron::Stop()
{
  {
    lock_guard<mutex> _x1( _M_l );

    RemoveState( CRON_ST_STARTED );
    cond.notify_one();
  }

  _thr->join();

  delete _thr;
  _thr = 0;
}

void __FIT_DECLSPEC Cron::EmptyStart()
{
  // do nothing
}

void __FIT_DECLSPEC Cron::EmptyStop()
{
  // do nothing
}

void Cron::_loop( Cron* p )
{
  // After creation cron loop (one per every Cron object),
  // this loop should exit in following cases:
  //   -) Cron receive Stop event (if it loop already started)
  //   -) Cron object destroyed
  // If Cron's container empty, this thread suspend, and can be alarmed
  // after Add (Cron entry) event.
  Cron& me = *p;

  me.PushState( CRON_ST_STARTED );

  while ( me.isState( CRON_ST_STARTED ) ) {
    unique_lock<mutex> lk( me._M_l );
    if ( me._M_c.empty() ) {
      me.cond.wait( lk, me.running );
      if ( !me.isState( CRON_ST_STARTED ) ) {
        break;
      }
    }
    // At this point _M_c should never be empty!

    if ( me._M_c.top().expired > get_system_time() ) {
      bool alarm = me.cond.timed_wait( lk, me._M_c.top().expired, me.ready );

      if ( !me.isState( CRON_ST_STARTED ) ) {
        break;
      }

      if ( me._M_c.empty() ) { // event removed while I wait?
        continue;
      }

      if ( alarm && (me._M_c.top().expired > get_system_time()) ) {
        continue;
      }
    }

    __CronEntry en = me._M_c.top(); // get and eject top cron entry
    me._M_c.pop();

    if ( me.is_avail( en.addr ) ) { // check if target abonent exist
      Event ev( en.code );
      ev.dest( en.addr );
      ev.value() = en.arg;
      me.Send( ev );
    } else { // do nothing, this lead to removing invalid abonent from Cron
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
    } else if ( en.count < en.n ) {
      me._M_c.push( en );
    }
  }
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
  __pack( s, static_cast<int64_t>( start.nanoseconds_since_epoch().count() ) );
  __pack( s, static_cast<int64_t>( period.count() ) );
  __pack( s, n );
  __pack( s, arg );
}

__FIT_DECLSPEC
void CronEntry::unpack( std::istream& s )
{
  __unpack( s, code );
  int64_t v;
  __unpack( s, v );
  start = std::tr2::system_time( 0, v );
  __unpack( s, v );
  period = std::tr2::nanoseconds( v );
  __unpack( s, n );
  __unpack( s, arg );
}

} // namespace stem

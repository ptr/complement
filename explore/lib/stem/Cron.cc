// -*- C++ -*-

/*
 * Copyright (c) 1998, 2002-2003, 2005-2006, 2008-2009, 2020
 * Petr Ovtchenkov
 * 
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 * 
 */

#include <config/feature.h>
#include "stem/Cron.h"
#include "stem/EvManager.h"
#include "stem/EDSEv.h"

#include <cmath>

namespace stem {

using namespace std;

Cron::Cron() :
    EventHandler(),
    running( *this ),
    ready( *this ),
    _thr( 0 ),
    _run( false ),
    _top_changed( false )
{
  this->enable();
}

Cron::Cron( const char *info ) :
    EventHandler( info ),
    running( *this ),
    ready( *this ),
    _thr( 0 ),
    _run( false ),
    _top_changed( false )
{
  this->enable();
}

Cron::Cron( addr_type id, const char *info ) :
    EventHandler( id, info ),
    running( *this ),
    ready( *this ),
    _thr( 0 ),
    _run( false ),
    _top_changed( false )
{
  this->enable();
}

Cron::~Cron()
{
  this->disable();

  if ( _thr != 0 ) {
    {
      lock_guard<mutex> _x1( _M_l );

      _run = false;
      cond.notify_one();
    }
    _thr->join();
    delete _thr;
    _thr = 0;
  }
}

void Cron::AddFirst( const Event_base<CronEntry>& entry )
{
  Add( entry );

  lock_guard<mutex> _x1( _M_l );

  _run = true;
  _thr = new thread( _loop, this );
  if ( !_thr->joinable() ) {
    _run = false;
  }

  PushState( CRON_ST_STARTED );
}

void Cron::Add( const Event_base<CronEntry>& entry )
{
  const CronEntry& ne = entry.value();
  __CronEntry en;

  en.start = ne.start;
  en.n = ne.n;
  en.ev = ne.ev;

  if ( en.ev.dest() == extbadaddr ) {
    en.ev.dest( entry.src() );
  }

  if ( en.n == 1 ) {
    en.period = chrono::nanoseconds();
  } else {
    en.period = ne.period;
    if ( en.n == 0 || en.period.count() == 0LL ) {
      return;
    }
  }

  chrono::time_point<chrono::steady_clock,chrono::nanoseconds> current = chrono::steady_clock::now();
  if ( en.start < current ) {
    en.start = current;
  }

  en.expired = en.start;
  en.count = 0;

  lock_guard<mutex> _x1( _M_l );
  _M_c.push( en );
  if ( _M_c.top() == en ) { // cron has entries, but new entry is on top
    _top_changed = true;
    cond.notify_one();      // so, we need wait it; if only one entry now,
  }                         // then notification also happen (wake-up)
}

// Remove cron entry if recipient address and event code match to request
void Cron::Remove( const Event_base<CronEntry>& entry )
{
  lock_guard<mutex> _x1( _M_l );

  const CronEntry& ne = entry.value();
  std::vector<value_type> tmp;

  tmp.reserve( _M_c.size() );

  while ( !_M_c.empty() ) {
    if ( _M_c.top().ev.src() != entry.value().ev.src() || _M_c.top().ev.dest() != entry.value().ev.dest() || _M_c.top().ev.code() != ne.ev.code() ) {
      tmp.push_back( _M_c.top() );
    } else {
      _top_changed = true; // may be not on top...
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
void Cron::RemoveArg( const Event_base<CronEntry>& entry )
{
  lock_guard<mutex> _x1( _M_l );

  const CronEntry& ne = entry.value();
  std::vector<value_type> tmp;

  tmp.reserve( _M_c.size() );

  while ( !_M_c.empty() ) {
    if ( _M_c.top().ev.src() != entry.value().ev.src() || _M_c.top().ev.dest() != entry.value().ev.dest() || _M_c.top().ev.code() != ne.ev.code() || _M_c.top().ev.value() != ne.ev.value() ) {
      tmp.push_back( _M_c.top() );
    } else {
      _top_changed = true; // may be not on top...
    }
    _M_c.pop();
  }

  while ( !tmp.empty() ) {
    _M_c.push( tmp.back() );
    tmp.pop_back();
  }

  cond.notify_one(); // in any case, remove I something or not
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
  chrono::time_point<chrono::steady_clock,chrono::nanoseconds> st;

  unique_lock<mutex> lk( me._M_l );

  while ( me._run ) {
    if ( me._M_c.empty() ) {
      me.cond.wait( lk, me.running );
      if ( !me._run ) {
        break;
      }
    }
    // At this point _M_c should never be empty!

    if (me._M_c.top().expired > chrono::steady_clock::now()) {
      st = me._M_c.top().expired;
      me._top_changed = false;
      bool alarm;
      try {
        alarm = me.cond.wait_until(lk, st, me.ready);

        if ( !me._run ) {
          break;
        }

        if ( me._M_c.empty() ) { // event removed while I wait?
          continue;
        }

        if (alarm && (me._M_c.top().expired > chrono::steady_clock::now())) {
          continue;
        }

        if ( me._top_changed ) {
          me._top_changed = false;
          if (me._M_c.top().expired > chrono::steady_clock::now()) {
            continue;
          }
        }
      }
      catch ( const system_error& ) { // Cron crash, destroyed or bad time?
        continue;
      }
    }

    __CronEntry en = me._M_c.top(); // get and eject top cron entry
    me._M_c.pop();

    // if target is in the same domain, we can check if ones exist
    if ( (en.ev.dest().first == EventHandler::domain()) && !me.is_avail( en.ev.dest().second ) ) { 
      continue; // do nothing, this lead to removing non-existent abonent from Cron
    }    
    me.Forward( en.ev ); // otherwise, forward event

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
END_RESPONSE_TABLE

__FIT_DECLSPEC
void CronEntry::pack( std::ostream& s ) const
{
  __pack(s, static_cast<int64_t>(start.time_since_epoch().count()));
  __pack(s, static_cast<int64_t>(period.count()));
  __pack( s, n );
  __pack( s, ev.code() );
  __pack( s, ev.dest() );
  __pack( s, ev.src() );
  __pack( s, ev.flags() );
  __pack( s, ev.value() );
}

__FIT_DECLSPEC
void CronEntry::unpack( std::istream& s )
{
  int64_t v;
  __unpack( s, v );
  start = std::chrono::time_point<chrono::steady_clock,chrono::nanoseconds>(std::chrono::nanoseconds(v));
  __unpack( s, v );
  period = std::chrono::nanoseconds(v);
  __unpack( s, n );
  stem::code_type c;
  __unpack( s, c );
  ev.code( c );
  stem::ext_addr_type a;
  __unpack( s, a );
  ev.dest( a );
  __unpack( s, a );
  ev.src( a );
  uint32_t f;
  __unpack( s, f );
  ev.resetf( f );
  __unpack( s, ev.value() );
}

void CronEntry::swap( CronEntry& e )
{
  std::swap( start, e.start );
  std::swap( period, e.period );
  std::swap( n, e.n );
  std::swap( ev, e.ev );
}

void __CronEntry::swap( __CronEntry& e )
{
  std::swap( expired, e.expired );

  std::swap( start, e.start );
  std::swap( period, e.period );

  std::swap( n, e.n );
  std::swap( count, e.count );
  std::swap( ev, e.ev );
}

} // namespace stem

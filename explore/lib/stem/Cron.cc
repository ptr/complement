// -*- C++ -*- Time-stamp: <00/02/16 15:05:02 ptr>

/*
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
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

#ident "$SunId$ %Q%"

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#pragma warning( disable : 4800 )
#endif

#ifdef WIN32
#  ifdef _DLL
#    define __EDS_DLL __declspec( dllexport )
#  else
#    define __EDS_DLL
#  endif
#else
#  define __EDS_DLL
#endif

#include <config/feature.h>
#include "EDS/Cron.h"
#include "EDS/EvManager.h"
#include "EDS/EDSEv.h"

#include <cmath>

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT __EDS_DLL
#endif

#define CRON_ST_STARTED 0x10

namespace EDS {

__EDS_DLL Cron::Cron() :
    EventHandler()
{
}

__EDS_DLL Cron::Cron( const char *info ) :
    EventHandler( info )
{
}

__EDS_DLL Cron::Cron( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

__EDS_DLL Cron::~Cron()
{
  if ( isState( CRON_ST_STARTED ) ) {
    Stop();
  }
  _thr.join();
}

void __EDS_DLL Cron::AddFirst( const Event_base<CronEntry>& entry )
{
  Add( entry );
  Start();
}

void __EDS_DLL Cron::Add( const Event_base<CronEntry>& entry )
{
  const CronEntry& ne = entry.value();
  __CronEntry en;

  en.code = ne.code;
  en.addr = entry.src();
  en.start.tv_sec = ne.start;
  en.period.tv_sec = ne.period.tv_sec;
  en.period.tv_nsec = ne.period.tv_nsec;
  en.n = ne.n;

  time_t current;
  if ( en.start.tv_sec < time( &current ) ) {
    en.start.tv_sec = current;
  }

  if ( en.n == 0 || (en.period.tv_sec == 0 && en.period.tv_nsec == 0 ) ||
       en.period.tv_nsec >= 1000000000 ) {
    return;
  }

  en.expired.tv_sec = en.start.tv_sec;
  // en.expired.tv_nsec = 0;
  en.count = 0;

  MT_REENTRANT( _M_l, _1 );
  _M_c.push( en );
  if ( _M_c.top() == en ) {
    cond.signal();
  }
}

// Remove cron entry if recipient address and event code match to request
void __EDS_DLL Cron::Remove( const Event_base<CronEntry>& entry )
{
  MT_REENTRANT( _M_l, _1 );
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

void __EDS_DLL Cron::Start()
{
  MT_REENTRANT( _M_l, _1 );
  if ( !_M_c.empty() ) { // start only if Cron queue not empty
    _thr.launch( _loop, this );
  }
}

void __EDS_DLL Cron::Stop()
{
  PopState();
  cond.signal();
}

void __EDS_DLL Cron::EmptyStart()
{
  // do nothing
}

void __EDS_DLL Cron::EmptyStop()
{
  // do nothing
}

int Cron::_loop( void *p )
{
  Cron& me = *reinterpret_cast<Cron *>(p);

  me.PushState( CRON_ST_STARTED );

  timespec abstime;
  int res;
  while ( me.isState( CRON_ST_STARTED ) ) {
    // At this point _M_c should never be empty!
    // If I detect that _M_c is empty, I or don't start, or
    // exit from this thread
    {
      MT_REENTRANT( me._M_l, _1 );
      if ( me._M_c.empty() ) {
        timespec t;
        t.tv_sec = 0;
        t.tv_nsec = 10000000; // 10 ms
        // make chance to PopState() from other thread...
        __impl::Thread::sleep( &t );
        continue; // break?
      }
      abstime = me._M_c.top().expired;
    }
    res = me.cond.wait_time( &abstime );
    if ( res > 0 ) { // time expired
      __CronEntry en;
      {
        MT_REENTRANT( me._M_l, _1 );
        if ( me._M_c.empty() ) {
          continue; // break?
        }
        en = me._M_c.top();
        me._M_c.pop();
      }

      Event ev( en.code );
      ev.dest( en.addr );
      me.Send( ev );

#ifndef __STL_LONG_LONG     
      double _next = en.start.tv_sec + en.start.tv_nsec * 1.0e-9 +
        (en.period.tv_sec + en.period.tv_nsec * 1.0e-9) * ++en.count;
      en.expired.tv_nsec = 1.0e9 * modf( _next, &_next );
      en.expired.tv_sec = _next;
#else
      long long _d = en.period.tv_nsec;
      _d *= ++en.count;
      _d += en.start.tv_nsec;
      en.expired.tv_sec = en.period.tv_sec;
      en.expired.tv_sec *= en.count;
      en.expired.tv_sec += en.start.tv_sec;
      en.expired.tv_sec += unsigned(_d / 1000000000);
      en.expired.tv_nsec = unsigned(_d % 1000000000);
#endif

      if ( en.n == CronEntry::infinite ) {
        en.start.tv_sec = en.expired.tv_sec;
        en.start.tv_nsec = en.expired.tv_nsec;        
        en.count = 0;
        MT_REENTRANT( me._M_l, _1 );
        me._M_c.push( en );
      } else if ( (en.count) < (en.n) ) { // This SC5.0 patch 107312-06 bug
        MT_REENTRANT( me._M_l, _1 );
        me._M_c.push( en );
      } else {
        MT_REENTRANT( me._M_l, _1 );
        if ( me._M_c.empty() ) {
          me.PopState();
        }
      }
    } else { // signaled or error
      // This occur if new record added in Cron queue, or
      // this is request for cron loop termination
      MT_REENTRANT( me._M_l, _1 );
      if ( me._M_c.empty() && me.isState(CRON_ST_STARTED) ) {
        // If Cron queue is empty, no needs in loop, I terminate this thread
        me.PopState();
      }
      // in case of Cron queue empty, PopState may be call outside this thread,
      // like Cron::Stop
    }
  }

  return 0;
}


DEFINE_RESPONSE_TABLE( Cron )
  EV_Event_base_T_(ST_NULL,EV_EDS_CRON_ADD,AddFirst,CronEntry)
  EV_Event_base_T_(CRON_ST_STARTED,EV_EDS_CRON_ADD,Add,CronEntry)
  EV_Event_base_T_(ST_NULL,EV_EDS_CRON_REMOVE,Remove,CronEntry)
  EV_VOID(ST_NULL,EV_EDS_CRON_START,Start)
  EV_VOID(CRON_ST_STARTED,EV_EDS_CRON_STOP,Stop)
  EV_VOID(CRON_ST_STARTED,EV_EDS_CRON_START,EmptyStart)
  EV_VOID(ST_NULL,EV_EDS_CRON_STOP,EmptyStop)
END_RESPONSE_TABLE

__EDS_DLL
void CronEntry::pack( std::ostream& s ) const
{
  __pack( s, code );
  __pack( s, start );
  __pack( s, period.tv_sec );
  __pack( s, period.tv_nsec );
  __pack( s, n );
}

__EDS_DLL
void CronEntry::net_pack( std::ostream& s ) const
{
  __net_pack( s, code );
  __net_pack( s, start );
  __net_pack( s, period.tv_sec );
  __net_pack( s, period.tv_nsec );
  __net_pack( s, n );
}

__EDS_DLL
void CronEntry::unpack( std::istream& s )
{
  __unpack( s, code );
  __unpack( s, start );
  __unpack( s, period.tv_sec );
  __unpack( s, period.tv_nsec );
  __unpack( s, n );
}

__EDS_DLL
void CronEntry::net_unpack( std::istream& s )
{
  __net_unpack( s, code );
  __net_unpack( s, start );
  __net_unpack( s, period.tv_sec );
  __net_unpack( s, period.tv_nsec );
  __net_unpack( s, n );
}

} // namespace EDS

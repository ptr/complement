// -*- C++ -*- Time-stamp: <99/10/14 22:04:42 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
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
  Start();
}

__EDS_DLL Cron::~Cron()
{
  _thr.join();
}

void __EDS_DLL Cron::Add( const Event_base<CronEntry>& entry )
{
//  Event_base<NameRecord> rs( EV_EDS_NM_LIST );
  const CronEntry& ne = entry.value();
  __CronEntry en;

  en.code = ne.code;
  en.addr = entry.src();
  en.start.tv_sec = ne.start;
  en.period.tv_sec = ne.period.tv_sec;
  en.period.tv_nsec = ne.period.tv_nsec;
  en.n = ne.n;

  if ( en.start.tv_sec == 0 ) {
    en.start.tv_sec = time( 0 );
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
#if 0
    // create timer before...
    itimerspec _timer;

    _timer.it_interval.tv_sec = 0;
    _timer.it_interval.tv_nsec = 0;

    _timer.it_value.tv_sec = en.tm_sec;
    _timer.it_value.tv_nsec = en.tm_nsec;

    int ii = timer_settime( _timerid, TIMER_ABSTIME, &_timer, 0 );
    cerr << "** " << ii << endl;
#else
    cond.signal();
#endif
  }

//  Send( Event_convert<NameRecord>()( rs ) );
}

void __EDS_DLL Cron::Remove( const Event_base<CronEntry>& entry )
{
//  Event_base<NameRecord> rs( EV_EDS_NM_LIST );

//  Send( Event_convert<NameRecord>()( rs ) );
}

void __EDS_DLL Cron::Start()
{
  _thr.launch( _loop, this );
}

void __EDS_DLL Cron::Stop()
{
  PopState();
  cond.signal(); // wrong
}

void __EDS_DLL Cron::EmptyStart()
{
}

void __EDS_DLL Cron::EmptyStop()
{
}

void alarm( int sig )
{
  cerr << "+++++++++++>>>> " << sig << endl;
}

extern "C" {
  typedef void(*sigev_notify_type)(sigval);
}

int Cron::_loop( void *p )
{
  Cron& me = *reinterpret_cast<Cron *>(p);

  me.PushState( CRON_ST_STARTED );
  me.cond.set( false );
#if 0
  struct sigevent {
        int             sigev_notify;   /* notification mode */
        int             sigev_signo;    /* signal number */
        union sigval    sigev_value;    /* signal value */
        void            (*sigev_notify_function)(union sigval);
        pthread_attr_t  *sigev_notify_attributes;
        int             __sigev_pad2;
  };
#endif

#if 0
  __impl::Thread::signal_handler( SIGALRM, SIG_PF(alarm) );
  __impl::Thread::block_signal( SIGALRM );
  __impl::Thread::unblock_signal( SIGINT );

  sigevent _sev;
  _sev.sigev_notify = SIGEV_SIGNAL;
  _sev.sigev_signo = SIGALRM;
  _sev.sigev_value.sival_int = 0;
  _sev.sigev_notify_function = 0; // (sigev_notify_type)alarm;
  _sev.sigev_notify_attributes = 0;

  int ii = timer_create( CLOCK_REALTIME, &_sev, &me._timerid );
  cerr << "$$ " << ii << endl;

  timespec tic;
  // tic.tv_sec  = int( 2 );
  // tic.tv_nsec = int( 0 );
  tic.tv_sec  = int( 10 );
  tic.tv_nsec = int( 100000000 );

  while ( me.cond.set() ) {
    nanosleep( &tic, 0 );
  }

  timer_delete( me._timerid );
#else
  timespec abstime;
  int res;
  while ( me.isState( CRON_ST_STARTED ) ) {
    MT_LOCK( me._M_l );
    if ( me._M_c.empty() ) {
      MT_UNLOCK( me._M_l );
      me.cond.try_wait();
      MT_LOCK( me._M_l );
      if ( me._M_c.empty() ) { // may be it's signal for stop
        MT_UNLOCK( me._M_l );
        continue;
      }
      // MT_UNLOCK( me._M_l );
    }
    // MT_UNLOCK( me._M_l );
    // MT_LOCK( me._M_l );
    abstime = me._M_c.top().expired;
    MT_UNLOCK( me._M_l );
    res = me.cond.wait_time( &abstime );
    if ( res > 0 ) { // time expired
      MT_LOCK( me._M_l );
      __CronEntry en = me._M_c.top();
      me._M_c.pop();
      MT_UNLOCK( me._M_l );

      Event ev( en.code );
      ev.dest( en.addr );
      cerr << "======= Send ======== " << en.addr << hex << "=" << en.code << endl;
      me.Send( ev );
     
      double _next = en.start.tv_sec + en.start.tv_nsec * 1.0e-9 +
        (en.period.tv_sec + en.period.tv_nsec * 1.0e-9) * ++en.count;
      en.expired.tv_nsec = 1.0e9 * modf( _next, &_next );
      en.expired.tv_sec = _next;

      if ( en.count < en.n ) {
        MT_LOCK( me._M_l );
        me._M_c.push( en );
        MT_UNLOCK( me._M_l );
      } else if ( en.n == static_cast<unsigned>(-1) ) {
        en.start.tv_sec = en.expired.tv_sec;
        en.start.tv_nsec = en.expired.tv_nsec;        
        en.count = 0;
        MT_LOCK( me._M_l );
        me._M_c.push( en );
        MT_UNLOCK( me._M_l );
      }
    }
  }
#endif
  cerr << "Thread terminated" << endl;

  return 0;
}


DEFINE_RESPONSE_TABLE( Cron )
  EV_Event_base_T_(ST_NULL,EV_EDS_CRON_ADD,Add,CronEntry)
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
  // __pack( s, addr );
  __pack( s, start );
  __pack( s, period.tv_sec );
  __pack( s, period.tv_nsec );
  __pack( s, n );
}

__EDS_DLL
void CronEntry::net_pack( std::ostream& s ) const
{
  __net_pack( s, code );
  // __net_pack( s, addr );
  __net_pack( s, start );
  __net_pack( s, period.tv_sec );
  __net_pack( s, period.tv_nsec );
  __net_pack( s, n );
}

__EDS_DLL
void CronEntry::unpack( std::istream& s )
{
  __unpack( s, code );
  // __pack( s, addr );
  __unpack( s, start );
  __unpack( s, period.tv_sec );
  __unpack( s, period.tv_nsec );
  __unpack( s, n );
}

__EDS_DLL
void CronEntry::net_unpack( std::istream& s )
{
  __net_unpack( s, code );
  // __net_unpack( s, addr );
  __net_unpack( s, start );
  __net_unpack( s, period.tv_sec );
  __net_unpack( s, period.tv_nsec );
  __net_unpack( s, n );
}

} // namespace EDS

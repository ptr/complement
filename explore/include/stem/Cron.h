// -*- C++ -*- Time-stamp: <00/05/26 10:54:31 ptr>

/*
 * Copyright (c) 1999-2000
 * ParallelGraphics
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

#ifndef __Cron_h
#define __Cron_h

#ident "$SunId$"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __IOSFWD__
#include <iosfwd>
#endif

#ifndef __EventHandler_h
#include <EDS/EventHandler.h>
#endif

#ifndef __EvPack_h
#include <EDS/EvPack.h>
#endif

#include <ctime>
#include <mt/xmt.h>

#include <queue>

namespace EDS {

#ifdef _WIN32
using std::timespec;
#endif

struct CronEntry :
   public __pack_base
{
    enum {
      immediate = 0,
      infinite = -1
    };

    CronEntry() :
        code( static_cast<code_type>(-1) ),
        // addr( badaddr ),
        start( immediate ),
        n( infinite )
      { period.tv_sec = 0; period.tv_nsec = 0; }

    CronEntry( const CronEntry& x ) :
        code( x.code ),
        start( x.start ),
        n( x.n )
      { period.tv_sec = x.period.tv_sec; period.tv_nsec = x.period.tv_nsec; }

    code_type code;
    time_t start;
    // time_t end;
    timespec period;
    unsigned n;

    virtual __PG_DECLSPEC void pack( __STD::ostream& s ) const;
    virtual __PG_DECLSPEC void net_pack( __STD::ostream& s ) const;
    virtual __PG_DECLSPEC void unpack( __STD::istream& s );
    virtual __PG_DECLSPEC void net_unpack( __STD::istream& s );
};

struct __CronEntry
{
    __CronEntry() :
        code( static_cast<code_type>(-1) ),
        addr( badaddr ),
        n( 0 ),
        count( 0 )
      {
        expired.tv_sec = 0; expired.tv_nsec = 0;
        start.tv_sec = 0; start.tv_nsec = 0;
        period.tv_sec = 0; period.tv_nsec = 0;
      }

    __CronEntry( const __CronEntry& x ) :
        code( x.code ),
        addr( x.addr ),
        n( x.n ),
        count( x.count )
      {
        expired.tv_sec = x.expired.tv_sec; expired.tv_nsec = x.expired.tv_nsec;
        start.tv_sec = x.start.tv_sec; start.tv_nsec = x.start.tv_nsec;
        period.tv_sec = x.period.tv_sec; period.tv_nsec = x.period.tv_nsec;
      }

    timespec expired;

    code_type code;
    addr_type addr;

    timespec start;
    timespec period;

    unsigned n;
    unsigned count;

#if 0
    operator <( const __CronEntry& __x ) const
      { return expired.tv_sec < __x.expired.tv_sec ? true :
                 expired.tv_sec > __x.expired.tv_sec ? false :
                    expired.tv_nsec < __x.expired.tv_nsec; }
    operator >( const __CronEntry& __x ) const
      { return expired.tv_sec > __x.expired.tv_sec ? true :
                 expired.tv_sec < __x.expired.tv_sec ? false :
                    expired.tv_nsec > __x.expired.tv_nsec; }
    operator ==( const __CronEntry& __x ) const
      { return expired.tv_sec == __x.expired.tv_sec &&
                 expired.tv_nsec == __x.expired.tv_nsec; }
#endif
};

inline
bool operator <( const __CronEntry& __l, const __CronEntry& __r )
{ return __l.expired.tv_sec < __r.expired.tv_sec ? true :
         __l.expired.tv_sec > __r.expired.tv_sec ? false :
         __l.expired.tv_nsec < __r.expired.tv_nsec; }

inline
bool operator >( const __CronEntry& __l, const __CronEntry& __r )
{ return __l.expired.tv_sec > __r.expired.tv_sec ? true :
         __l.expired.tv_sec < __r.expired.tv_sec ? false :
         __l.expired.tv_nsec > __r.expired.tv_nsec; }

inline
bool operator ==( const __CronEntry& __l, const __CronEntry& __r )
{ return __l.expired.tv_sec == __r.expired.tv_sec &&
         __l.expired.tv_nsec == __r.expired.tv_nsec; }

class Cron :
    public EventHandler
{
  public:
    __PG_DECLSPEC Cron();
    explicit __PG_DECLSPEC Cron( const char * );
    explicit __PG_DECLSPEC Cron( addr_type id, const char *info = 0 );

    __PG_DECLSPEC ~Cron();

    __PG_DECLSPEC void Add( const Event_base<CronEntry>& );
    __PG_DECLSPEC void AddFirst( const Event_base<CronEntry>& );
    __PG_DECLSPEC void Remove( const Event_base<CronEntry>& );
    __PG_DECLSPEC void Start();
    __PG_DECLSPEC void Stop();

    __PG_DECLSPEC void EmptyStart();
    __PG_DECLSPEC void EmptyStop();

  private:
    static int _loop( void * );

    __impl::Thread _thr;
    __impl::Condition cond;

    typedef __CronEntry value_type;
    typedef __STD::priority_queue<value_type,
      __STD::vector<value_type>,
      __STD::greater<value_type> > container_type;

    container_type _M_c;
    __impl::Mutex _M_l;

  private:
    DECLARE_RESPONSE_TABLE( Cron, EventHandler );
};

} // namespace EDS

#endif // __Cron_h

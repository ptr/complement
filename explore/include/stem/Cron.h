// -*- C++ -*- Time-stamp: <99/10/15 12:21:44 ptr>

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

#ifndef __Cron_h
#define __Cron_h

#ident "$SunId$ %Q%"

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

#ifndef __EDS_DLL
#  if defined( WIN32 ) && defined( _MSC_VER )
#    define __EDS_DLL __declspec( dllimport )
#  else
#    define __EDS_DLL
#  endif
#endif

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT __EDS_DLL
#endif

namespace EDS {

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

    virtual __EDS_DLL void pack( std::ostream& s ) const;
    virtual __EDS_DLL void net_pack( std::ostream& s ) const;
    virtual __EDS_DLL void unpack( std::istream& s );
    virtual __EDS_DLL void net_unpack( std::istream& s );
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
};

class Cron :
    public EventHandler
{
  public:
    __EDS_DLL Cron();
    explicit __EDS_DLL Cron( const char * );
    explicit __EDS_DLL Cron( addr_type id, const char *info = 0 );

    __EDS_DLL ~Cron();

    __EDS_DLL void Add( const Event_base<CronEntry>& );
    __EDS_DLL void AddFirst( const Event_base<CronEntry>& );
    __EDS_DLL void Remove( const Event_base<CronEntry>& );
    __EDS_DLL void Start();
    __EDS_DLL void Stop();

    __EDS_DLL void EmptyStart();
    __EDS_DLL void EmptyStop();

  private:
    static int _loop( void * );

    __impl::Thread _thr;
    __impl::Condition cond;

    typedef __CronEntry value_type;
    typedef std::priority_queue<value_type,
      std::vector<value_type>,
      std::greater<value_type> > container_type;

    container_type _M_c;
    __impl::Mutex _M_l;

  private:
    DECLARE_RESPONSE_TABLE( Cron, EventHandler );
};

} // namespace EDS

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT
#endif

#endif // __Cron_h

// -*- C++ -*- Time-stamp: <06/07/20 10:30:46 ptr>

/*
 * Copyright (c) 1998, 2002, 2003, 2005
 * Petr Ovtchenkov
 * 
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.1
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

#ifndef __stem_Cron_h
#define __stem_Cron_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifndef __IOSFWD__
#include <iosfwd>
#endif

#ifndef __stem_EventHandler_h
#include <stem/EventHandler.h>
#endif

#ifndef __stem_EvPack_h
#include <stem/EvPack.h>
#endif

#include <ctime>
#include <mt/xmt.h>

#include <queue>

namespace stem {

struct CronEntry :
   public __pack_base
{
    enum {
      immediate = 0U,
      infinite = ~0U
    };

    CronEntry() :
        code( badcode ),
        n( static_cast<unsigned>(infinite) ),
        arg( 0 )
      {
        start.tv_sec = immediate; start.tv_nsec = 0;
        period.tv_sec = 0; period.tv_nsec = 0;
      }

    CronEntry( const CronEntry& x ) :
        code( x.code ),
        n( x.n ),
        arg( x.arg )
      {
        start.tv_sec = x.start.tv_sec; start.tv_nsec = x.start.tv_nsec;
        period.tv_sec = x.period.tv_sec; period.tv_nsec = x.period.tv_nsec;
      }

    code_type code;
    // time_t start;
    timespec start;
    // time_t end;
    timespec period;
    uint32_t n;
    uint32_t arg;

    virtual __FIT_DECLSPEC void pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void net_pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void unpack( std::istream& s );
    virtual __FIT_DECLSPEC void net_unpack( std::istream& s );
};

struct __CronEntry
{
    __CronEntry() :
        code( badcode ),
        addr( badaddr ),
        n( 0 ),
        count( 0 ),
        arg( 0 )
      {
        expired.tv_sec = 0; expired.tv_nsec = 0;
        start.tv_sec = 0; start.tv_nsec = 0;
        period.tv_sec = 0; period.tv_nsec = 0;
      }

    __CronEntry( const __CronEntry& x ) :
        code( x.code ),
        addr( x.addr ),
        n( x.n ),
        count( x.count ),
        arg( x.arg )
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
    unsigned arg;

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
    __FIT_DECLSPEC Cron();
    explicit __FIT_DECLSPEC Cron( const char * );
    explicit __FIT_DECLSPEC Cron( addr_type id, const char *info = 0 );

    __FIT_DECLSPEC ~Cron();

    __FIT_DECLSPEC void Add( const Event_base<CronEntry>& );
    __FIT_DECLSPEC void AddFirst( const Event_base<CronEntry>& );
    __FIT_DECLSPEC void Remove( const Event_base<CronEntry>& );
    __FIT_DECLSPEC void RemoveArg( const Event_base<CronEntry>& );
    __FIT_DECLSPEC void Start();
    __FIT_DECLSPEC void Stop();

    __FIT_DECLSPEC void EmptyStart();
    __FIT_DECLSPEC void EmptyStop();

  private:
    static int _loop( void * );

    xmt::Thread _thr;
    xmt::Condition cond;

    typedef __CronEntry value_type;
    typedef std::priority_queue<value_type,
      std::vector<value_type>,
      std::greater<value_type> > container_type;

    container_type _M_c;
    xmt::Mutex _M_l;

  private:
    DECLARE_RESPONSE_TABLE( Cron, EventHandler );
};

} // namespace stem

#endif // __Cron_h

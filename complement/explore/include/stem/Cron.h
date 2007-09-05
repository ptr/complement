// -*- C++ -*- Time-stamp: <07/09/05 01:08:54 ptr>

/*
 * Copyright (c) 1998, 2002, 2003, 2005, 2007
 * Petr Ovtchenkov
 * 
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __stem_Cron_h
#define __stem_Cron_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <iosfwd>

#ifndef __stem_EventHandler_h
#include <stem/EventHandler.h>
#endif

#ifndef __stem_EvPack_h
#include <stem/EvPack.h>
#endif

#include <ctime>
#include <mt/xmt.h>
#include <mt/time.h>

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
        start = immediate;
        period = 0;
      }

    CronEntry( const CronEntry& x ) :
        code( x.code ),
        n( x.n ),
        arg( x.arg )
      {
        start = x.start;
        period = x.period;
      }

    code_type code;
    // time_t start;
    xmt::timespec start;
    // time_t end;
    xmt::timespec period;
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
        expired = 0;
        start = 0;
        period = 0;
      }

    __CronEntry( const __CronEntry& x ) :
        code( x.code ),
        addr( x.addr ),
        n( x.n ),
        count( x.count ),
        arg( x.arg )
      {
        expired = x.expired;
        start = x.start;
        period = x.period;
      }

    xmt::timespec expired;

    code_type code;
    addr_type addr;

    xmt::timespec start;
    xmt::timespec period;

    unsigned n;
    unsigned count;
    unsigned arg;
};

inline
bool operator <( const __CronEntry& __l, const __CronEntry& __r )
{ return __l.expired < __r.expired; }

inline
bool operator >( const __CronEntry& __l, const __CronEntry& __r )
{ return __l.expired > __r.expired; }

inline
bool operator ==( const __CronEntry& __l, const __CronEntry& __r )
{ return __l.expired == __r.expired; }

inline
bool operator !=( const __CronEntry& __l, const __CronEntry& __r )
{ return __l.expired != __r.expired; }

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
    static xmt::Thread::ret_t _loop( void * );

    xmt::Thread _thr;
    xmt::condition cond;

    typedef __CronEntry value_type;
    typedef std::priority_queue<value_type,
      std::vector<value_type>,
      std::greater<value_type> > container_type;

    container_type _M_c;
    xmt::mutex _M_l;

  private:
    DECLARE_RESPONSE_TABLE( Cron, EventHandler );
};

} // namespace stem

#endif // __Cron_h

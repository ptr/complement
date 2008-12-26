// -*- C++ -*- Time-stamp: <08/12/05 22:47:49 ptr>

/*
 * Copyright (c) 1998, 2002, 2003, 2005, 2007, 2008
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
#include <mt/mutex>
#include <mt/thread>
#include <mt/date_time>

#include <queue>
#include <string>

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
        start(), // immediate
        period(),
        n( static_cast<uint32_t>(infinite) ),
        arg()
      { }

    CronEntry( const CronEntry& x ) :
        code( x.code ),
        start( x.start ),
        period( x.period ),
        n( x.n ),
        arg( x.arg )
      { }

    code_type code;
    // time_t start;
    std::tr2::system_time start;
    // time_t end;
    std::tr2::nanoseconds period;
    uint32_t n;
    std::string arg;

    virtual __FIT_DECLSPEC void pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void net_pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void unpack( std::istream& s );
    virtual __FIT_DECLSPEC void net_unpack( std::istream& s );
};

struct __CronEntry
{
    __CronEntry() :
        expired(),
        code( badcode ),
        addr( badaddr ),
        start(),
        period(),
        n( 0 ),
        count( 0 ),
        arg()
      { }

    __CronEntry( const __CronEntry& x ) :
        expired( x.expired ),
        code( x.code ),
        addr( x.addr ),
        start( x.start ),
        period( x.period ),
        n( x.n ),
        count( x.count ),
        arg( x.arg )
      {  }

    std::tr2::system_time expired;

    code_type code;
    addr_type addr;

    std::tr2::system_time start;
    std::tr2::nanoseconds period;

    unsigned n;
    unsigned count;
    std::string arg;
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
    static void _loop( Cron* );

    enum {
      CRON_ST_STARTED   = 0x10
    };

    struct _running
    {
        _running( Cron& c ) :
            me( c )
          { }

        bool operator()() const
          { return !me._M_c.empty() || !me.isState( CRON_ST_STARTED ); }

        Cron& me;
    } running;

    std::tr2::thread* _thr;

    typedef __CronEntry value_type;
    typedef std::priority_queue<value_type,
      std::vector<value_type>,
      std::greater<value_type> > container_type;

    container_type _M_c;
    std::tr2::mutex _M_l;
    std::tr2::condition_variable cond;

  private:
    DECLARE_RESPONSE_TABLE( Cron, EventHandler );
};

} // namespace stem

#endif // __Cron_h

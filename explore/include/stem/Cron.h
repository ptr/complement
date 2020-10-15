// -*- C++ -*-

/*
 * Copyright (c) 1998, 2002, 2003, 2005, 2007-2009, 2020
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

#include <config/feature.h>

#include <iosfwd>

#ifndef __stem_EventHandler_h
#include <stem/EventHandler.h>
#endif

#ifndef __stem_EvPack_h
#include <stem/EvPack.h>
#endif

#include <ctime>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

#include <queue>
#include <string>
#include <utility>

namespace stem {

struct CronEntry :
   public __pack_base
{
    enum {
      immediate = 0U,
      infinite = ~0U
    };

    CronEntry() :
        start(), // immediate
        period(),
        n( static_cast<uint32_t>(infinite) ),
        ev()
      { }

    CronEntry( const CronEntry& x ) :
        start( x.start ),
        period( x.period ),
        n( x.n ),
        ev( x.ev )
      { }

    std::chrono::time_point<chrono::steady_clock,std::chrono::nanoseconds> start;
    std::chrono::nanoseconds period;
    uint32_t n;
    stem::Event ev;

    virtual __FIT_DECLSPEC void pack( std::ostream& s ) const;
    virtual __FIT_DECLSPEC void unpack( std::istream& s );

    void swap( CronEntry& );
};

struct __CronEntry
{
    __CronEntry() :
        expired(),
        start(),
        period(),
        n( 0 ),
        count( 0 ),
        ev()
      { }

    __CronEntry( const __CronEntry& x ) :
        expired( x.expired ),
        start( x.start ),
        period( x.period ),
        n( x.n ),
        count( x.count ),
        ev( x.ev )
      {  }

    void swap( __CronEntry& );

    std::chrono::time_point<chrono::steady_clock,std::chrono::nanoseconds> expired;

    std::chrono::time_point<chrono::steady_clock,std::chrono::nanoseconds> start;
    std::chrono::nanoseconds period;

    unsigned n;
    unsigned count;
    stem::Event ev;
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
          { return !me._M_c.empty() || !me._run; }

        Cron& me;
    } running;

    struct _ready
    {
        _ready( Cron& c ) :
            me( c )
          { }

        bool operator()() const
          {
            return (!me._run ||
                    me._top_changed ||
                    me._M_c.empty() ||
                    (me._M_c.top().expired <= std::chrono::steady_clock::now())) ?
              true : false;
          }

        Cron& me;
    } ready;

    std::thread* _thr;

    typedef __CronEntry value_type;
    typedef std::priority_queue<value_type,
      std::vector<value_type>,
      std::greater<value_type> > container_type;

    container_type _M_c;
    std::mutex _M_l;
    std::condition_variable cond;
    bool _run;
    bool _top_changed;

    friend struct _running;
    friend struct _ready;

  private:
    DECLARE_RESPONSE_TABLE( Cron, EventHandler );
};

} // namespace stem

namespace std {

template <>
inline void swap( stem::CronEntry& l, stem::CronEntry& r )
{ l.swap(r); }

template <>
inline void swap( stem::__CronEntry& l, stem::__CronEntry& r )
{ l.swap(r); }

} // namespace std

#endif // __Cron_h

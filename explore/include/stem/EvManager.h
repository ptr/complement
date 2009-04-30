// -*- C++ -*- Time-stamp: <09/04/30 10:53:55 ptr>

/*
 * Copyright (c) 1995-1999, 2002-2003, 2005-2006, 2009
 * Petr Ovtchenkov
 * 
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __stem_EvManager_h
#define __stem_EvManager_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <stdint.h>

#include <string>
// #include <map>
#include <deque>
#include <list>
#include <vector>
#include <queue>
#include <functional>

#include <mt/mutex>
#include <mt/thread>
#include <mt/condition_variable>
#include <mt/uid.h>
#include <mt/uidhash.h>

#include <stem/Event.h>
#include <stem/EventHandler.h>
#include <ostream>

#ifdef STLPORT
#  include <unordered_map>
#  include <unordered_set>
// #  include <hash_map>
// #  include <hash_set>
// #  define __USE_STLPORT_HASH
#  define __USE_STLPORT_TR1
#else
#  if defined(__GNUC__) && (__GNUC__ < 4)
#    include <ext/hash_map>
#    include <ext/hash_set>
#    define __USE_STD_HASH
#  else
#    include <tr1/unordered_map>
#    include <tr1/unordered_set>
#    define __USE_STD_TR1
#  endif
#endif

namespace std {

// for priority_queue

template<>
struct less< std::pair<unsigned,stem::EventHandler*> >
{
    bool operator()(const std::pair<unsigned,stem::EventHandler*>& __x, const std::pair<unsigned,stem::EventHandler*>& __y) const
      { return __x.first < __y.first; }
};

}

namespace stem {

class EvManager
{
  private:
    typedef std::pair<unsigned,EventHandler*> weighted_handler;
#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<addr_type,std::priority_queue<weighted_handler> > local_heap_type;
    typedef std::hash_map<std::string,std::list<addr_type> > info_heap_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<addr_type,std::priority_queue<weighted_handler> > local_heap_type;
    typedef __gnu_cxx::hash_map<std::string,std::list<addr_type> > info_heap_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<addr_type,std::priority_queue<weighted_handler> > local_heap_type;
    typedef std::tr1::unordered_map<std::string,std::list<addr_type> > info_heap_type;
#endif

  public:

    enum traceflags {
      notrace = 0,
      tracenet = 1,
      tracedispatch = 2,
      tracefault = 4
    };

    typedef std::deque< Event > queue_type;

    __FIT_DECLSPEC EvManager();
    __FIT_DECLSPEC ~EvManager();

    __FIT_DECLSPEC addr_type Subscribe( EventHandler *object, const std::string& info );
    __FIT_DECLSPEC addr_type Subscribe( EventHandler *object, const char *info = 0 );
    __FIT_DECLSPEC addr_type SubscribeID( addr_type id, EventHandler *object,
                                          const std::string& info );
    __FIT_DECLSPEC addr_type SubscribeID( addr_type id, EventHandler *object,
                                          const char *info = 0 );
    __FIT_DECLSPEC bool Unsubscribe( addr_type id );

    bool is_avail( addr_type id ) const
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_heap );
        return unsafe_is_avail(id);
      }

    void change_announce( addr_type id, const std::string& info )
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );
        unsafe_annotate( id, info );
      }

    void change_announce( addr_type id, const char *info )
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );
        unsafe_annotate( id, info );
      }

    __FIT_DECLSPEC void push( const Event& e );

    __FIT_DECLSPEC void Remove( void * );
    __FIT_DECLSPEC std::ostream& dump( std::ostream& ) const;

    void settrf( unsigned f );
    void unsettrf( unsigned f );
    void resettrf( unsigned f );
    void cleantrf();
    unsigned trflags() const;
    void settrs( std::ostream * );

  protected:
    bool unsafe_is_avail( addr_type id ) const
      {
        return heap.find( id ) != heap.end();
      }

    void unsafe_annotate( addr_type id, const std::string& info )
      {
        iheap[ info ].push_back( id );
      }

    void unsafe_annotate( addr_type id, const char *info )
      {
        iheap[ std::string(info) ].push_back( id );
      }

  private:
    struct _not_empty
    {
        _not_empty( EvManager& m ) :
            me( m )
          { }

        bool operator()() const
          { return !me.in_ev_queue.empty() || me._dispatch_stop; }

        EvManager& me;
    } not_empty;


    void Send( const Event& e );
    __FIT_DECLSPEC void unsafe_Remove( void * );

    local_heap_type heap;   // address -> EventHandler *
    info_heap_type  iheap;  // address -> info string (both local and external)

    queue_type in_ev_queue;
    queue_type out_ev_queue;

    static void _Dispatch( EvManager* );

    bool not_finished();

    bool _dispatch_stop;

    std::tr2::mutex _lock_heap;
    std::tr2::mutex _lock_iheap;

    std::tr2::mutex _lock_queue;
    std::tr2::condition_variable _cnd_queue;

    static std::string inv_key_str;
    std::tr2::mutex _lock_tr;
    unsigned _trflags;
    std::ostream *_trs;

    std::tr2::thread _ev_queue_thr;

    friend class Names;
    friend class NetTransportMgr;
    friend class NetTransport_base;
    friend class NetTransport;
};

} // namespace stem

#ifdef __USE_STLPORT_HASH
#  undef __USE_STLPORT_HASH
#endif
#ifdef __USE_STD_HASH
#  undef __USE_STD_HASH
#endif
#ifdef __USE_STLPORT_TR1
#  undef __USE_STLPORT_TR1
#endif
#ifdef __USE_STD_TR1
#  undef __USE_STD_TR1
#endif

#endif // __stem_EvManager_h

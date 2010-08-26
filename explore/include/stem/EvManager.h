// -*- C++ -*- Time-stamp: <10/07/16 21:23:26 ptr>

/*
 * Copyright (c) 1995-1999, 2002-2003, 2005-2006, 2009-2010
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

namespace stem {
namespace detail {

typedef std::pair<int,std::pair<int,EventHandler*> > weighted_handler_type;

} // namespace detail
} // namespace stem

namespace std {

// for priority_queue

template<>
struct less<stem::detail::weighted_handler_type>
{
    bool operator()(const stem::detail::weighted_handler_type& __x, const stem::detail::weighted_handler_type& __y) const
      { return __x.first > __y.first; }
};

} // namespace std

#if defined(__USE_STLPORT_HASH) || defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
#  define __HASH_NAMESPACE std
#endif
#if defined(__USE_STD_HASH)
#  define __HASH_NAMESPACE __gnu_cxx
#endif

namespace __HASH_NAMESPACE {

#ifdef __USE_STD_TR1
namespace tr1 {
#endif

template <>
struct hash<stem::EventHandler*>
{
    size_t operator()(const stem::EventHandler* __x) const
      { return reinterpret_cast<size_t>(__x); }
};

#ifdef __USE_STD_TR1
}
#endif

} // namespace __HASH_NAMESPACE

namespace stem {

class EvManager
{
  private:
    typedef stem::detail::weighted_handler_type weighted_handler_type;
    typedef std::priority_queue<weighted_handler_type> handlers_type;
    typedef std::list<addr_type> addr_collection_type;
#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<addr_type,handlers_type> local_heap_type;
    typedef std::hash_map<std::string,addr_collection_type> info_heap_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<addr_type,handlers_type> local_heap_type;
    typedef __gnu_cxx::hash_map<std::string,addr_collection_type> info_heap_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<addr_type,handlers_type> local_heap_type;
    typedef std::tr1::unordered_map<std::string,addr_collection_type> info_heap_type;
#endif

  public:

    enum traceflags {
      notrace = 0,
      tracenet = 1,
      tracedispatch = 2,
      tracefault = 4,
      tracesubscr = 8,
      tracetime = 16,
      tracesend = 32
    };

    enum objectflags {
      remote = 1,
      nosend = 2
    };

    __FIT_DECLSPEC EvManager();
    __FIT_DECLSPEC ~EvManager();

    void Subscribe( const addr_type& id, EventHandler* object, int nice = 0 )
      {
        std::tr2::lock_guard<std::tr2::rw_mutex> lk( _lock_heap );
        unsafe_Subscribe( id, object, nice );
      }

    void Subscribe( const addr_type& id, EventHandler* object, const std::string& info, int nice = 0 )
      {
        std::tr2::lock_guard<std::tr2::rw_mutex> lk( _lock_heap );
        std::tr2::lock_guard<std::tr2::mutex> lk_( _lock_iheap );
        unsafe_Subscribe( id, object, nice );
        unsafe_annotate( id, info );
      }

    void Subscribe( const addr_type& id, EventHandler* object, const char* info, int nice = 0 )
      {
        std::tr2::lock_guard<std::tr2::rw_mutex> lk( _lock_heap );
        std::tr2::lock_guard<std::tr2::mutex> lk_( _lock_iheap );
        unsafe_Subscribe( id, object, nice );

        if ( info == 0 || info[0] == 0 ) {
          return;
        }
        unsafe_annotate( id, std::string( info ) );
      }

    template <class Iterator>
    void Subscribe( Iterator first, Iterator last, EventHandler* obj, int nice = 0 )
      {
        std::tr2::lock_guard<std::tr2::rw_mutex> _x1( _lock_heap );

        while ( first != last ) {
          unsafe_Subscribe( *first, obj, nice );
          ++first;
        }
      }

    template <class Iterator>
    void Subscribe( Iterator first, Iterator last, EventHandler* obj, const std::string& info, int nice = 0 )
      {
        std::tr2::lock_guard<std::tr2::rw_mutex> _x1( _lock_heap );
        std::tr2::lock_guard<std::tr2::mutex> lk_( _lock_iheap );

        while ( first != last ) {
          unsafe_Subscribe( *first, obj, nice );
          unsafe_annotate( *first, info );
          ++first;
        }
      }

    template <class Iterator>
    void Subscribe( Iterator first, Iterator last, EventHandler* obj, const char* info, int nice = 0 )
      {
        bool ann = (info == 0) || (info[0] == 0)? false : true;
        std::string _info;
        
        if ( ann ) {
          _info = info;
        }

        std::tr2::lock_guard<std::tr2::rw_mutex> _x1( _lock_heap );
        std::tr2::lock_guard<std::tr2::mutex> lk_( _lock_iheap );

        while ( first != last ) {
          unsafe_Subscribe( *first, obj, nice );
          if ( ann ) {
            unsafe_annotate( *first, _info );
          }
          ++first;
        }
      }

    void Unsubscribe( const addr_type& id, EventHandler* obj );

    template <class Iterator>
    void Unsubscribe( Iterator first, Iterator last, EventHandler* obj )
      {
        std::tr2::lock_guard<std::tr2::rw_mutex> _x1( _lock_heap );
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );

        while ( first != last ) {
          unsafe_Unsubscribe( *first++, obj );
        }
      }

    bool is_avail( const addr_type& id ) const
      {
        std::tr2::basic_read_lock<std::tr2::rw_mutex> lk( _lock_heap );
        return unsafe_is_avail( id );
      }

    void annotate( const addr_type& id, const std::string& info )
      {
        if ( info.empty() ) {
          return;
        }
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );
        unsafe_annotate( id, info );
      }

    void annotate( const addr_type& id, const char* info )
      {
        if ( info == 0 || info[0] == 0 ) {
          return;
        }
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );
        unsafe_annotate( id, std::string( info ) );
      }

    __FIT_DECLSPEC void push( const Event& e );

    __FIT_DECLSPEC std::ostream& dump( std::ostream& ) const;

    void settrf( unsigned f );
    void unsettrf( unsigned f );
    void resettrf( unsigned f );
    void cleantrf();
    unsigned trflags() const;
    std::ostream* settrs( std::ostream* );

    // This is UUID, not address
    const xmt::uuid_type& self_id() const
      { return _id; }

    static void start_queue() { };
    static void stop_queue() { };

  protected:
    void unsafe_Subscribe( const addr_type& id, EventHandler* object, int nice = 0 );
    void unsafe_Unsubscribe( const addr_type& id, EventHandler* );
    // void cache_clear( EventHandler* );

    bool unsafe_is_avail( const addr_type& id ) const
      { return heap.find( id ) != heap.end(); }

    void unsafe_annotate( const addr_type& id, const std::string& info );

  private:
    const xmt::uuid_type _id;

    local_heap_type heap;   // address -> EventHandler *
    info_heap_type  iheap;  // address -> info string (both local and external)

    struct worker
    {
      worker( EvManager* _mgr ) :
          mgr( _mgr ),
          not_empty( *this ),
          thr( new std::tr2::thread( worker::_loop, this ) )
        { }

      ~worker() {
        if ( thr != NULL ) {
          thr->join();
          delete thr;
        }
      }

      class queue_condition {
        public:
          queue_condition( worker& _w ) :
              w( _w )
            { }

          bool operator()() const
            { return !w.events.empty() || w.mgr->_dispatch_stop; }

        private:
          worker& w;
      } not_empty;

      std::list<Event> events;
      EvManager* mgr;

      std::tr2::mutex lock;
      std::tr2::condition_variable cnd;
      std::tr2::thread* thr;

      static void _loop( worker* );
    };


    bool _dispatch_stop;

    std::tr2::rw_mutex _lock_heap;
    std::tr2::mutex _lock_iheap;

    static std::string inv_key_str;
    std::tr2::mutex _lock_tr;
    unsigned _trflags;
    std::ostream *_trs;

    const int n_threads;
    std::vector< worker* > workers;

    friend class Names;
    friend class NetTransportMgr;
    friend class NetTransport_base;
    friend class NetTransport;
    friend class EventHandler::Init;
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

// -*- C++ -*- Time-stamp: <2011-06-08 20:27:53 ptr>

/*
 * Copyright (c) 1995-1999, 2002-2003, 2005-2006, 2009-2011
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

#include <config/feature.h>

#include <stdint.h>

#include <string>
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

#if defined(STLPORT) || defined(__FIT_CPP_0X)
#  include <unordered_map>
#  include <unordered_set>
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

class NetTransport_base;

class EvManager
{
  private:
    typedef xmt::uuid_type edge_id_type;

    union target_type {
        void* object;
        domain_type domain;
    };

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
    typedef std::unordered_map<addr_type,target_type> local_heap_type;
    typedef std::unordered_map<std::string,addr_collection_type> info_heap_type;
    typedef std::unordered_map<domain_type,std::list<edge_id_type> > vertex_container_type;
    typedef std::unordered_map<edge_id_type,std::pair<std::pair<domain_type,domain_type>,unsigned> > edge_container_type;
    typedef std::unordered_map<edge_id_type,void*> bridge_container_type;
    typedef std::unordered_map<domain_type,edge_id_type> pi_type;
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

#ifdef __FIT_CPP_0X
    EvManager( const EvManager& ) = delete;
    EvManager& operator =( const EvManager& ) = delete;
#else
  private:
    EvManager( const EvManager& );
    EvManager& operator =( const EvManager& );

  public:
#endif

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

    void start_queue();
    void stop_queue();

    static unsigned int working_threads;

  protected:
    void unsafe_Subscribe( const addr_type& id, EventHandler* object, int nice = 0 );
    void unsafe_Unsubscribe( const addr_type& id, EventHandler* );

    bool unsafe_is_avail( const addr_type& id ) const
      { return heap.find( id ) != heap.end(); }

    void unsafe_annotate( const addr_type& id, const std::string& info );

  private:
    local_heap_type heap;   // address -> EventHandler *
    info_heap_type  iheap;  // address -> info string (both local and external)
    vertex_container_type vertices;
    edge_container_type edges;
    bridge_container_type bridges;
    pi_type gate; // predecessors

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

    unsigned int n_threads;
    std::vector< worker* > workers;

    friend class Names;
    friend class NetTransportMgr;
    friend class NetTransport_base;
    friend class NetTransport;
    friend class EventHandler::Init;

  protected:
    EvManager::edge_id_type bridge( NetTransport_base*, const domain_type& );
    void connectivity( const edge_id_type& eid, const domain_type& u, const domain_type& v, unsigned w, NetTransport_base* b );

  private:
    void route_calc();
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

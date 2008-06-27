// -*- C++ -*- Time-stamp: <08/06/27 12:35:36 ptr>

/*
 * Copyright (c) 1995-1999, 2002, 2003, 2005, 2006
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

#include <mt/mutex>
#include <mt/thread>
#include <mt/condition_variable>
#include <mt/uid.h>

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

typedef void * transport_entry;

struct transport
{
    enum kind_type {
      unknown = -1,
      socket_tcp = 0
    };

    transport() :
        link( 0 ),
        metric( 0 ),
        kind( unknown )
      { }

    transport( transport_entry l, kind_type k, int m = 0 ) :
        link( l ),
        metric( m ),
        kind( k )
      { }

    transport( const transport& t ) :
        link( t.link ),
        metric( t.metric ),
        kind( t.kind )
      { }

    transport_entry link;
    int metric;

    kind_type kind;
};

inline bool operator <( const transport& l, const transport& r )
{ return l.metric < r.metric; }

} // namespace detail

class EvManager
{
  private:
#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<addr_type,EventHandler *> local_heap_type;
    typedef std::hash_map<addr_type,std::string>    info_heap_type;

    typedef std::hash_map<addr_type,gaddr_type> ext_uuid_heap_type;

    typedef std::hash_multimap<gaddr_type,std::pair<addr_type,detail::transport> > uuid_tr_heap_type;
    typedef std::hash_multimap<detail::transport_entry,gaddr_type> tr_uuid_heap_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<addr_type,EventHandler *> local_heap_type;
    typedef __gnu_cxx::hash_map<addr_type,std::string>    info_heap_type;

    typedef __gnu_cxx::hash_map<addr_type,gaddr_type> ext_uuid_heap_type;

    typedef __gnu_cxx::hash_multimap<gaddr_type,std::pair<addr_type,detail::transport> > uuid_tr_heap_type;
    typedef __gnu_cxx::hash_multimap<detail::transport_entry,gaddr_type> tr_uuid_heap_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<addr_type,EventHandler *> local_heap_type;
    typedef std::tr1::unordered_map<addr_type,std::string>    info_heap_type;

    typedef std::tr1::unordered_map<addr_type,gaddr_type> ext_uuid_heap_type;

    typedef std::tr1::unordered_multimap<gaddr_type,std::pair<addr_type,detail::transport> > uuid_tr_heap_type;
    typedef std::tr1::unordered_multimap<detail::transport_entry,gaddr_type> tr_uuid_heap_type;
#endif

#if 0
    typedef std::map<addr_type,EventHandler *> local_heap_type;
    typedef std::map<addr_type,std::string>    info_heap_type;

    typedef std::map<addr_type,gaddr_type> ext_uuid_heap_type;

    typedef std::multimap<gaddr_type,std::pair<addr_type,detail::transport> > uuid_tr_heap_type;
    typedef std::multimap<detail::transport_entry,gaddr_type> tr_uuid_heap_type;
#endif

    static bool tr_compare( const std::pair<gaddr_type,std::pair<addr_type,detail::transport> >& l, const std::pair<gaddr_type,std::pair<addr_type,detail::transport> >& r )
      { return l.second.second < r.second.second; }

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
    __FIT_DECLSPEC addr_type SubscribeRemote( const detail::transport& tr,
                                              const gaddr_type& addr,
                                              const std::string& info );
    __FIT_DECLSPEC addr_type SubscribeRemote( const detail::transport& tr,
                                              const gaddr_type& addr,
                                              const char *info = 0 );
    __FIT_DECLSPEC addr_type SubscribeRemote( const gaddr_type& addr,
                                              const std::string& info );
    __FIT_DECLSPEC addr_type SubscribeRemote( const gaddr_type& addr,
                                              const char *info = 0 );
    __FIT_DECLSPEC bool Unsubscribe( addr_type id );
    __FIT_DECLSPEC addr_type reflect( const gaddr_type& addr ) const;
    __FIT_DECLSPEC gaddr_type reflect( addr_type addr ) const;

    bool is_avail( addr_type id ) const
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_heap );
        return unsafe_is_avail(id);
      }

    const std::string who_is( addr_type id ) const
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );
        return unsafe_who_is( id );
      }

    const std::string annotate( addr_type id ) const
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );
        return unsafe_annotate( id );
      }

    void change_announce( addr_type id, const std::string& info )
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );
        unsafe_change_announce( id, info );
      }

    void change_announce( addr_type id, const char *info )
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_iheap );
        unsafe_change_announce( id, info );
      }

    __FIT_DECLSPEC const detail::transport& transport( addr_type object_id ) const;

    void push( const Event& e )
      {
        std::tr2::lock_guard<std::tr2::mutex> lk( _lock_queue );
        in_ev_queue.push_back( e );
        _cnd_queue.notify_one();
      }

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
        if ( id & extbit ) {
          return _ex_heap.find( id ) != _ex_heap.end();
        }
        return heap.find( id ) != heap.end();
      }

    const std::string& unsafe_who_is( addr_type id ) const
      {
        info_heap_type::const_iterator i = iheap.find( id );
        return i == iheap.end() ? inv_key_str : (*i).second;
      }
    const std::string& unsafe_annotate( addr_type id ) const
      {
        info_heap_type::const_iterator i = iheap.find( id );
        return i == iheap.end() ? inv_key_str : (*i).second;
      }

    void unsafe_change_announce( addr_type id, const std::string& info )
      {
        info_heap_type::iterator i = iheap.find( id );
        if ( i != iheap.end() ) {
          i->second = info;
        } else {
          iheap[id] = info;
        }
      }

    void unsafe_change_announce( addr_type id, const char *info )
      {
        info_heap_type::iterator i = iheap.find( id );
        if ( i != iheap.end() ) {
          if ( info != 0 ) {
            i->second = info;
          } else {
            i->second.clear();
          }
        } else if ( info != 0 ) {
          iheap[id] = info;
        }
      }

  private:
    void Send( const Event& e );
    __FIT_DECLSPEC void unsafe_Remove( void * );

    addr_type create_unique();
    addr_type create_unique_x();

    const addr_type _low;
    const addr_type _high;
    addr_type _id;

    local_heap_type heap;   // address -> EventHandler *
    info_heap_type  iheap;  // address -> info string (both local and external)

    ext_uuid_heap_type _ex_heap; // address -> global address
    uuid_tr_heap_type _tr_heap;  // global address -> address, transport
    tr_uuid_heap_type _ch_heap;  // transport channel -> global address

    queue_type in_ev_queue;
    queue_type out_ev_queue;

    const addr_type _x_low;
    const addr_type _x_high;
    addr_type _x_id;

    static void _Dispatch( EvManager* );

    bool not_finished();

    bool _dispatch_stop;

    std::tr2::mutex _lock_heap;
    std::tr2::mutex _lock_iheap;
    std::tr2::mutex _lock_xheap;

    std::tr2::mutex _lock_queue;
    std::tr2::condition_variable _cnd_queue;

    static std::string inv_key_str;
    std::tr2::mutex _lock_tr;
    unsigned _trflags;
    std::ostream *_trs;

    std::tr2::spinlock _ev_queue_dispatch_guard;
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

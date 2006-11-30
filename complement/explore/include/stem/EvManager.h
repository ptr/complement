// -*- C++ -*- Time-stamp: <06/11/30 22:33:25 ptr>

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
#include <map>
#include <queue>

#include <mt/xmt.h>
#include <mt/uid.h>

#include <stem/Event.h>
#include <stem/EventHandler.h>
#include <ostream>

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
    typedef std::map<addr_type,EventHandler *> local_heap_type;
    typedef std::map<addr_type,std::string>    info_heap_type;

    typedef std::map<gaddr_type,addr_type> uuid_ext_heap_type;
    typedef std::map<addr_type,gaddr_type> ext_uuid_heap_type;

    typedef std::multimap<gaddr_type,detail::transport> uuid_tr_heap_type;
    typedef std::multimap<detail::transport_entry,gaddr_type> tr_uuid_heap_type;

    static bool tr_compare( const std::pair<gaddr_type,detail::transport>& l, const std::pair<gaddr_type,detail::transport>& r )
      { return l.second < r.second; }

  public:

    enum traceflags {
      notrace = 0,
      tracenet = 1,
      tracedispatch = 2,
      tracefault = 4
    };

    typedef std::queue< Event > queue_type;

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
        MT_REENTRANT( _lock_heap, _x1 );
        return unsafe_is_avail(id);
      }

    const std::string who_is( addr_type id ) const
      {
        MT_REENTRANT( _lock_iheap, _x1 );
        return unsafe_who_is( id );
      }

    const std::string annotate( addr_type id ) const
      {
        MT_REENTRANT( _lock_iheap, _x1 );
        return unsafe_annotate( id );
      }

    void change_announce( addr_type id, const std::string& info )
      {
        MT_REENTRANT( _lock_iheap, _x1 );
        unsafe_change_announce( id, info );
      }

    void change_announce( addr_type id, const char *info )
      {
        MT_REENTRANT( _lock_iheap, _x1 );
        unsafe_change_announce( id, info );
      }

    __FIT_DECLSPEC const detail::transport& transport( addr_type object_id ) const;

    void push( const Event& e )
      {
        MT_REENTRANT( _lock_queue, _x1 );
        in_ev_queue.push( e );
        _cnd_queue.set( true );
      }

    __FIT_DECLSPEC void Remove( void * );
    __FIT_DECLSPEC std::ostream& dump( std::ostream& ) const;

    static void settrf( unsigned f );
    static void unsettrf( unsigned f );
    static void resettrf( unsigned f );
    static void cleantrf();
    static unsigned trflags();
    static void settrs( std::ostream * );

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

    uuid_ext_heap_type _ui_heap; // gloabal address -> address
    ext_uuid_heap_type _ex_heap; // address -> global address
    uuid_tr_heap_type _tr_heap;  // global address -> transport
    tr_uuid_heap_type _ch_heap;  // transport channel -> global address

    queue_type in_ev_queue;
    queue_type out_ev_queue;

    const addr_type _x_low;
    const addr_type _x_high;
    addr_type _x_id;

    static xmt::Thread::ret_code _Dispatch( void * );

    bool not_finished();

    bool _dispatch_stop;

    xmt::Thread _ev_queue_thr;
    xmt::Spinlock _ev_queue_dispatch_guard;

    xmt::Mutex _lock_heap;
    xmt::Mutex _lock_iheap;
    xmt::Mutex _lock_xheap;

    xmt::Mutex _lock_queue;
    xmt::Condition _cnd_queue;

    static std::string inv_key_str;
    static xmt::Mutex _lock_tr;
    static unsigned _trflags;
    static std::ostream *_trs;

    friend class Names;
    friend class NetTransportMgr;
};

} // namespace stem

#endif // __stem_EvManager_h

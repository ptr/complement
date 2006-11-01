// -*- C++ -*- Time-stamp: <06/10/12 15:10:18 ptr>

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

#include <string>
#include <map>
#include <queue>

#ifndef __stem_Event_h
#include <stem/Event.h>
#endif

#ifndef __stem_EventHandler_h
#include <stem/EventHandler.h>
#endif

#ifndef __stem_EvSession_h
#include <stem/EvSession.h>
#endif

#ifndef __XMT_H
#include <mt/xmt.h>
#endif

#ifndef __SOCKSTREAM__
#include <sockios/sockstream>
#endif

namespace stem {

class NetTransport_base;
class NetTransport;
class NetTransportMgr;

struct __Remote_Object_Entry
{
    __Remote_Object_Entry() :
        key( 0 ),
        channel( 0 )
      { }

    __Remote_Object_Entry( key_type __k, NetTransport_base *__c ) :
        key( __k ),
        channel( __c )
      { }

    key_type key;
    NetTransport_base *channel;
};

struct __Object_Entry
{
    __Object_Entry() :
        ref( 0 ),
        remote( 0 )
      { }

    ~__Object_Entry()
      { delete remote; }

    void addremote( key_type key, NetTransport_base *channel )
      { remote = new __Remote_Object_Entry( key, channel ); }
        
    EventHandler *ref;  // system dependent? for Win may be WND HANDLER?    
    std::string info; // even IDL interface...
    __Remote_Object_Entry *remote;
 // string location; // if ref invalid;
 // int refcount;    // references on object
};

#ifdef _MSC_VER
} // namespace stem
namespace std {
typedef  stem::__Object_Entry __Object_Entry;
} // namespace std
namespace stem {
#endif

class EvManager
{
  public:
//    typedef std::map<key_type,__Object_Entry,std::less<key_type>> heap_type;
    typedef std::map<key_type,__Object_Entry> heap_type;
    typedef std::queue< Event > queue_type;

    __FIT_DECLSPEC EvManager();
    __FIT_DECLSPEC ~EvManager();

    __FIT_DECLSPEC addr_type Subscribe( EventHandler *object, const std::string& info );
    __FIT_DECLSPEC addr_type Subscribe( EventHandler *object, const char *info = 0 );
    __FIT_DECLSPEC addr_type SubscribeID( addr_type id, EventHandler *object,
                                    const std::string& info );
    __FIT_DECLSPEC addr_type SubscribeID( addr_type id, EventHandler *object,
                                    const char *info = 0 );
    __FIT_DECLSPEC addr_type SubscribeRemote( NetTransport_base *channel,
                                         addr_type rmkey,
                                         const std::string& info );
    __FIT_DECLSPEC addr_type SubscribeRemote( NetTransport_base *channel,
                                         addr_type rmkey,
                                         const char *info = 0 );
    __FIT_DECLSPEC bool Unsubscribe( addr_type id );

    bool is_avail( addr_type id ) const
      {
        MT_REENTRANT( _lock_heap, _x1 );
        return unsafe_is_avail(id);
      }

    const std::string who_is( addr_type id ) const
      {
        MT_REENTRANT( _lock_heap, _x1 );
        return unsafe_who_is( id );
      }

    const std::string annotate( addr_type id ) const
      {
        MT_REENTRANT( _lock_heap, _x1 );
        return unsafe_annotate( id );
      }

    void change_announce( addr_type id, const std::string& info )
      {
        MT_REENTRANT( _lock_heap, _x1 );
        unsafe_change_announce( id, info );
      }

    void change_announce( addr_type id, const char *info )
      {
        MT_REENTRANT( _lock_heap, _x1 );
        unsafe_change_announce( id, info );
      }

    __FIT_DECLSPEC NetTransport_base *transport( addr_type object_id ) const;

    void push( const Event& e )
      {
        MT_REENTRANT( _lock_queue, _x1 );
        in_ev_queue.push( e );
        _cnd_queue.set( true );
      }

    __FIT_DECLSPEC void Remove( NetTransport_base * );

  protected:
    bool unsafe_is_avail( addr_type id ) const
      { return heap.find( id ) != heap.end(); }

    const std::string& unsafe_who_is( addr_type id ) const
      {
        heap_type::const_iterator i = heap.find( id );
        return i == heap.end() ? inv_key_str : (*i).second.info;
      }
    const std::string& unsafe_annotate( addr_type id ) const
      {
        heap_type::const_iterator i = heap.find( id );
        return i == heap.end() ? inv_key_str : (*i).second.info;
      }

    void unsafe_change_announce( addr_type id, const std::string& info )
      {
        heap_type::iterator i = heap.find( id );
        if ( i != heap.end() ) {
          i->second.info = info;
        }
      }

    void unsafe_change_announce( addr_type id, const char *info )
      {
        heap_type::iterator i = heap.find( id );
        if ( i != heap.end() ) {
          i->second.info = info;
        }
      }

  private:
    void Send( const Event& e );
    __FIT_DECLSPEC void unsafe_Remove( NetTransport_base * );

    addr_type create_unique();
    addr_type create_unique_x();

    const addr_type _low;
    const addr_type _high;
    addr_type _id;
    heap_type heap;
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
    xmt::Mutex _lock_queue;
    xmt::Condition _cnd_queue;

    static std::string inv_key_str;

    friend class Names;
};

} // namespace stem

#endif // __stem_EvManager_h

// -*- C++ -*- Time-stamp: <05/12/30 00:14:38 ptr>

/*
 * Copyright (c) 1995-1999, 2002, 2003, 2005
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

    const string& who_is( addr_type id ) const
      {
        MT_REENTRANT( _lock_heap, _x1 );
        return unsafe_who_is( id );
      }

    const string& annotate( addr_type id ) const
      {
        MT_REENTRANT( _lock_heap, _x1 );
        return unsafe_annotate( id );
      }

    __FIT_DECLSPEC key_type sid( addr_type object_id ) const;
    __FIT_DECLSPEC NetTransport_base *transport( addr_type object_id ) const;

    void push( const Event& e )
      {
        MT_REENTRANT( _lock_queue, _x1 );
        in_ev_queue.push( e );
        if ( out_ev_queue.size() == 0 ) {
          _ev_queue_thr.resume();
        }
      }

    __FIT_DECLSPEC void Remove( NetTransport_base * );

  protected:
    bool unsafe_is_avail( addr_type id ) const
      { return heap.find( id ) != heap.end(); }

    const string& unsafe_who_is( addr_type id ) const
      {
        heap_type::const_iterator i = heap.find( id );
        return i == heap.end() ? inv_key_str : (*i).second.info;
      }
    const string& unsafe_annotate( addr_type id ) const
      {
        heap_type::const_iterator i = heap.find( id );
        return i == heap.end() ? inv_key_str : (*i).second.info;
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

    static int _Dispatch( void * );

    xmt::Thread _ev_queue_thr;
    xmt::Condition _ev_queue_cond;

    xmt::Mutex _lock_heap;
    xmt::Mutex _lock_queue;

    static std::string inv_key_str;

    friend class Names;
};

} // namespace stem

#endif // __stem_EvManager_h

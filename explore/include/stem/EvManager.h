// -*- C++ -*- Time-stamp: <99/10/15 18:49:26 ptr>

/*
 *
 * Copyright (c) 1995-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __EvManager_h
#define __EvManager_h

#ident "$SunId$ %Q%"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <map>

#ifndef __Event_h
#include <EDS/Event.h>
#endif

#ifndef __EventHandler_h
#include <EDS/EventHandler.h>
#endif

#ifndef __EvSession_h
#include <EDS/EvSession.h>
#endif

#ifndef __XMT_H
#include <mt/xmt.h>
#endif

#ifndef __SOCKSTREAM__
#include <sockios/sockstream>
#endif

namespace EDS {

class NetTransport_base;
class NetTransport;
class NetTransportMgr;

struct __Remote_Object_Entry
{
    __Remote_Object_Entry() :
        key( 0 ),
        channel( 0 )
      { }

    __Remote_Object_Entry( Event::key_type __k, NetTransport_base *__c ) :
        key( __k ),
        channel( __c )
      { }

    Event::key_type key;
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

    void addremote( Event::key_type key, NetTransport_base *channel )
      { remote = new __Remote_Object_Entry( key, channel ); }
        
    EventHandler *ref;  // system dependent? for Win may be WND HANDLER?    
    std::string info; // even IDL interface...
    __Remote_Object_Entry *remote;
 // string location; // if ref invalid;
 // int refcount;    // references on object
};

#ifdef _MSC_VER
} // namespace EDS
namespace std {
typedef  EDS::__Object_Entry __Object_Entry;
} // namespace std
namespace EDS {
#endif

class EvManager
{
  public:
    typedef Event::key_type key_type;
    typedef std::map<key_type,__Object_Entry,std::less<key_type>,
                              __STL_DEFAULT_ALLOCATOR(__Object_Entry) > heap_type;

    __EDS_DLL EvManager();

    __EDS_DLL addr_type Subscribe( EventHandler *object, const std::string& info );
    __EDS_DLL addr_type Subscribe( EventHandler *object, const char *info = 0 );
    __EDS_DLL addr_type SubscribeID( addr_type id, EventHandler *object,
                                    const std::string& info );
    __EDS_DLL addr_type SubscribeID( addr_type id, EventHandler *object,
                                    const char *info = 0 );
    __EDS_DLL addr_type SubscribeRemote( NetTransport_base *channel,
                                         addr_type rmkey,
                                         const std::string& info );
    __EDS_DLL addr_type SubscribeRemote( NetTransport_base *channel,
                                         addr_type rmkey,
                                         const char *info = 0 );
    __EDS_DLL bool Unsubscribe( addr_type id );

    bool is_avail( addr_type id ) const
      { return heap.find( id ) != heap.end(); }

    const string& who_is( addr_type id ) const
      {
        heap_type::const_iterator i = heap.find( id );
        return i == heap.end() ? inv_key_str : (*i).second.info;
      }

    const string& annotate( addr_type id ) const
      {
        heap_type::const_iterator i = heap.find( id );
        return i == heap.end() ? inv_key_str : (*i).second.info;
      }

    __EDS_DLL key_type sid( addr_type object_id ) const;
    __EDS_DLL NetTransport_base *transport( addr_type object_id ) const;

    void Send( const Event& e );

  private:
    addr_type create_unique();
    addr_type create_unique_x();

    void Remove( NetTransport_base * );

    const addr_type _low;
    const addr_type _high;
    addr_type _id;
    heap_type heap;

    const addr_type _x_low;
    const addr_type _x_high;
    addr_type _x_id;

    __impl::Mutex _lock_heap;

    static std::string inv_key_str;

    friend class NetTransport_base;
    friend class NetTransport;
    friend class NetTransportMgr;
    friend class NetTransportMP;

    friend class Names;
};

} // namespace EDS

#endif // __EvManager_h

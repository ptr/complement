// -*- C++ -*- Time-stamp: <99/09/08 13:23:10 ptr>
#ifndef __EvManager_h
#define __EvManager_h

#ident "$SunId$ %Q%"

#include <string>
#include <map>

#ifndef __Event_h
#include <Event.h>
#endif

#ifndef __EventHandler_h
#include <EventHandler.h>
#endif

#ifndef __EvSession_h
#include <EvSession.h>
#endif

#include <xmt.h>
#include <sockstream>

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

    key_type sid( addr_type object_id ) const;

    void Send( const Event& e );

  private:
    addr_type create_unique();

    void Remove( NetTransport_base * );
    void Dispatch( const Event& e );

    const addr_type _low;
    const addr_type _high;
    addr_type _id;
    heap_type heap;

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

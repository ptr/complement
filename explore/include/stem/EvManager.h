// -*- C++ -*- Time-stamp: <99/04/16 14:42:33 ptr>
#ifndef __EvManager_h
#define __EvManager_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

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

    EvManager() :
        _low( __Event_Base::beglocaddr ),
        _high( __Event_Base::endlocaddr ),
        _id( _low )
      { }

    key_type Subscribe( EventHandler *object, const std::string& info )
      {
        MT_REENTRANT( _lock_heap, _1 );
        key_type id = create_unique();
        EDS::__Object_Entry& record = heap[id];
        record.ref = object;
        record.info = info;

        return id;
      }

    key_type SubscribeID( key_type id, EventHandler *object, const std::string& info )
      {
        MT_REENTRANT( _lock_heap, _1 );
        if ( (id & Event::extbit) || is_avail( id ) ) {
          return Event::badaddr;
        }
        EDS::__Object_Entry& record = heap[id];
        record.ref = object;
        record.info = info;

        return id;
      }

    key_type SubscribeRemote( NetTransport_base *channel, const key_type& rmkey, const std::string& info )
      {
        MT_REENTRANT( _lock_heap, _1 );
        key_type id = create_unique() | Event::extbit;
        EDS::__Object_Entry& record = heap[id];
        // record.ref = object;
        record.info = info;
        __stl_assert( channel != 0 );
        record.addremote( rmkey, channel );

        return id;
      }

    bool Unsubscribe( const key_type& id )
      {
        // MT_REENTRANT( _lock_heap, _1 );
        heap.erase( /* (const heap_type::key_type&)*/ id );
        return true; // may be here check object's reference count
      }

    bool is_avail( const key_type& id ) const
      { return heap.find( id ) != heap.end(); }

    const string& who_is( const key_type& id ) const
      {
        heap_type::const_iterator i = heap.find( id );
        if ( i == heap.end() ) {
          return inv_key_str;
        }
        return (*i).second.info;
      }

    EvSessionManager::key_type sid( const key_type& object_id ) const;

    void Send( const Event& e, const key_type& src_id = Event::badaddr );

  private:
    key_type create_unique();

    void Remove( NetTransport_base * );
    void Dispatch( const Event& e );

    const key_type _low;
    const key_type _high;
    key_type _id;
    heap_type heap;

    __impl::Mutex _lock_heap;

    static std::string inv_key_str;

    friend class NetTransport_base;
    friend class NetTransport;
    friend class NetTransportMgr;
    friend class NetTransportMP;
};

} // namespace EDS

#endif // __EvManager_h

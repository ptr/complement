// -*- C++ -*- Time-stamp: <08/12/12 00:46:37 ptr>

/*
 *
 * Copyright (c) 1995-1999, 2002, 2003, 2005-2008
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include "stem/EvManager.h"
#include "stem/NetTransport.h"
#include <iomanip>
#include <mt/mutex>

// #include <typeinfo>

namespace stem {

using namespace std;
using namespace std::tr2;

const addr_type badaddr      = 0xffffffff;
const code_type badcode      = 0xffffffff;
const addr_type extbit       = 0x80000000;
const addr_type default_addr = 0x00000000;
const addr_type ns_addr      = 0x00000001;
const addr_type janus_addr   = 0x00000002;

const addr_type beglocaddr = 0x00000100;
const addr_type endlocaddr = 0x3fffffff;
const addr_type begextaddr = extbit;
const addr_type endextaddr = 0xbfffffff;

std::string EvManager::inv_key_str( "invalid key" );

__FIT_DECLSPEC EvManager::EvManager() :
    not_empty( *this ),
    _low( beglocaddr ),
    _high( endlocaddr ),
    _id( _low ),
    _x_low( begextaddr ),
    _x_high( endextaddr ),
    _x_id( _x_low ),
    _dispatch_stop( false ),
    _trflags( 0 ),
    _trs( 0 ),
    _ev_queue_thr( _Dispatch, this )
{
  // _cnd_queue.set( false );
  // _ev_queue_thr.launch( _Dispatch, this );
}

__FIT_DECLSPEC EvManager::~EvManager()
{
  {
    lock_guard<mutex> lk( _lock_queue );
    _dispatch_stop = true;
    _cnd_queue.notify_one();
  }

  _ev_queue_thr.join();
}

bool EvManager::not_finished()
{
  lock_guard<mutex> lk( _lock_queue );
  return !_dispatch_stop;
}

void EvManager::_Dispatch( EvManager* p )
{
  EvManager& me = *p;
  mutex& lq = me._lock_queue;
  queue_type& in_ev_queue = me.in_ev_queue;
  queue_type& out_ev_queue = me.out_ev_queue;

  while ( me.not_finished() ) {
    {
      unique_lock<mutex> lk( lq );
      in_ev_queue.swap( out_ev_queue );
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(me._lock_tr);
        if ( me._trs != 0 && me._trs->good() && (me._trflags & tracedispatch) ) {
          ios_base::fmtflags f = me._trs->flags( ios_base::hex | ios_base::showbase );
          *me._trs << "EvManager queues swapped" << endl;
          me._trs->flags( f );
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE
    }
    
    while ( !out_ev_queue.empty() ) {
      me.Send( out_ev_queue.front() );
      out_ev_queue.pop_front();
    }
    {
      unique_lock<mutex> lk( lq );
      me._cnd_queue.wait( lk, me.not_empty );
    }
  }

#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(me._lock_tr);
    if ( me._trs != 0 && me._trs->good() && (me._trflags & tracedispatch) ) {
      ios_base::fmtflags f = me._trs->flags( ios_base::hex | ios_base::showbase );
      *me._trs << "EvManager Dispatch loop finished" << endl;
      me._trs->flags( f );
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  // try process the rest of queue, if not empty
  {
    unique_lock<mutex> lk( lq );
    in_ev_queue.swap( out_ev_queue );
  }
  while ( !out_ev_queue.empty() ) {
    me.Send( out_ev_queue.front() );
    out_ev_queue.pop_front();
  }

#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(me._lock_tr);
    if ( me._trs != 0 && me._trs->good() && (me._trflags & tracedispatch) ) {
      ios_base::fmtflags f = me._trs->flags( ios_base::hex | ios_base::showbase );
      *me._trs << "EvManager stop Dispatch" << endl;
      me._trs->flags( f );
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE
}

__FIT_DECLSPEC
addr_type EvManager::Subscribe( EventHandler *object, const std::string& info )
{
  addr_type id;
  {
    lock_guard<mutex> lk( _lock_heap );
    id = create_unique();
    heap[id] = object;
  }
  {
    lock_guard<mutex> lk( _lock_xheap );
    gaddr_type& gaddr = _ex_heap[id];
    gaddr.hid = xmt::hostid();
    gaddr.pid = std::tr2::getpid();
    gaddr.addr = id;
  }
  
  lock_guard<mutex> lk( _lock_iheap );
  iheap[id] = info;

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::Subscribe( EventHandler *object, const char *info )
{
  return info ? Subscribe( object, string(info) ) : Subscribe( object, string() );
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeID( addr_type id, EventHandler *object,
                                  const std::string& info )
{
  if ( (id & extbit) ) {
    return badaddr;
  } else {
    lock_guard<mutex> _x1( _lock_heap );
    if ( unsafe_is_avail( id ) ) {
      return badaddr;
    }
    heap[id] = object;
  }
  {
    lock_guard<mutex> lk( _lock_xheap );
    gaddr_type& gaddr = _ex_heap[id];
    gaddr.hid = xmt::hostid();
    gaddr.pid = std::tr2::getpid();
    gaddr.addr = id;
  }

  lock_guard<mutex> _x1( _lock_iheap );
  iheap[id] = info;

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeID( addr_type id, EventHandler *object,
                                  const char *info )
{
  return info ? SubscribeID( id, object, string(info) ) : SubscribeID( id, object, string() );
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeRemote( const detail::transport& tr,
                                      const gaddr_type& addr,
                                      const std::string& info )
{
  addr_type id;
  {
    lock_guard<mutex> _x1( _lock_xheap );
    id = create_unique_x();
    _ex_heap[id] = addr;
    _tr_heap.insert( make_pair( addr, make_pair( id, tr ) ) );
    _ch_heap.insert( make_pair( tr.link, addr ) );
  }
  {
    lock_guard<mutex> _x1( _lock_iheap );
    iheap[id] = info;
  }

  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeRemote( const detail::transport& tr,
                                      const gaddr_type& addr,
                                      const char *info )
{
  return info ? SubscribeRemote( tr, addr, string(info) ) : SubscribeRemote( tr, addr, string() );
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeRemote( const gaddr_type& addr,
                                      const std::string& info )
{
  addr_type id;
  if ( addr.hid == xmt::hostid() && addr.pid == std::tr2::getpid() ) { // local
    if ( addr.addr & extbit ) { // may be transit object
      lock_guard<mutex> lk( _lock_xheap );
      pair<uuid_tr_heap_type::const_iterator,uuid_tr_heap_type::const_iterator> range = _tr_heap.equal_range( addr );
      if ( range.first != range.second ) { // transport present
        return min_element( range.first, range.second, tr_compare )->second.first;
      }
    } else { // may be local object
      lock_guard<mutex> lk( _lock_heap );
      local_heap_type::const_iterator i = heap.find( addr.addr );
      if ( i != heap.end() ) {
        return i->first;
      }
    }
    return badaddr; // don't know what I can made
  } else { // foreign object
    lock_guard<mutex> lk( _lock_xheap );
    pair<uuid_tr_heap_type::const_iterator,uuid_tr_heap_type::const_iterator> tr_range = _tr_heap.equal_range( addr );
    if ( tr_range.first != tr_range.second ) { // transport present
      return min_element( tr_range.first, tr_range.second, tr_compare )->second.first;
    }

    gaddr_type peer_zero( addr );
    peer_zero.addr = default_addr;
    pair<uuid_tr_heap_type::const_iterator,uuid_tr_heap_type::const_iterator> range = _tr_heap.equal_range( peer_zero );
    if ( range.first == range.second ) { // no transport
      return badaddr;
    }
    id = create_unique_x();
    for ( uuid_tr_heap_type::const_iterator j = range.first; j != range.second; ++j ) {
      _tr_heap.insert( make_pair( addr, make_pair( id, j->second.second ) ) ); // all available transports
      _ch_heap.insert( make_pair( j->second.second.link, addr ) );
    }
    _ex_heap[id] = addr;
  }
  {
    lock_guard<mutex> lk( _lock_iheap );
    iheap[id] = info;
  }
  return id;
}

__FIT_DECLSPEC
addr_type EvManager::SubscribeRemote( const gaddr_type& addr,
                                      const char *info )
{
  return info ? SubscribeRemote( addr, string(info) ) : SubscribeRemote( addr, string() );
}

__FIT_DECLSPEC
bool EvManager::Unsubscribe( addr_type id )
{
  if ( (id & extbit) ) {
    lock_guard<mutex> _x1( _lock_xheap );
    gaddr_type& addr = _ex_heap[id];
      
    pair<uuid_tr_heap_type::iterator,uuid_tr_heap_type::iterator> range = _tr_heap.equal_range( addr );
    for ( uuid_tr_heap_type::iterator i = range.first; i != range.second; ) {
      pair<tr_uuid_heap_type::iterator,tr_uuid_heap_type::iterator> ch_range = _ch_heap.equal_range( i->second.second.link );
      for ( tr_uuid_heap_type::iterator j = ch_range.first; j != ch_range.second; ) {
        if ( j->second == i->first ) {
          _ch_heap.erase( j++ );
          continue;
        }
        ++j;
      }
      if ( i->second.first == id ) {
        _tr_heap.erase( i++ );
        continue;
      }
      ++i;
    }
    _ex_heap.erase( id );
  } else {
    lock_guard<mutex> _x1( _lock_heap );
    heap.erase( id );

    // Notify remotes?
  }
  lock_guard<mutex> _x1( _lock_iheap );
  iheap.erase( id );

  return true;
}

__FIT_DECLSPEC
addr_type EvManager::reflect( const gaddr_type& addr ) const
{
  if ( addr.hid == xmt::hostid() && addr.pid == std::tr2::getpid() ) {
    // this host, this process
    if ( (addr.addr & extbit) == 0 ) { // looks like local object
      lock_guard<mutex> _x1( _lock_heap );
      local_heap_type::const_iterator l = heap.find( addr.addr );
      if ( l != heap.end() ) {
        return addr.addr; // l->first
      }
    }
  }
#if 0
  else if ( addr.hid == gaddr_type() && addr.pid == -1 ) {
    // this host, special case:
    // peer don't know host ids, used as access to 'standard' services and initial
    // communication
    if ( (addr.addr & extbit) == 0 && addr.addr <= _low ) {
      scoped_lock _x1( _lock_heap );
      local_heap_type::const_iterator l = heap.find( addr.addr );
      if ( l != heap.end() ) {
        return addr.addr; // l->first
      }
    }
    // disable external objects here; restrict to 'standard' services
    return badaddr;
  }
#endif

  lock_guard<mutex> _x1( _lock_xheap );
  pair<uuid_tr_heap_type::const_iterator,uuid_tr_heap_type::const_iterator> range = _tr_heap.equal_range( addr );
  if ( range.first != range.second ) { // transport present
    return min_element( range.first, range.second, tr_compare )->second.first;
  }
  return badaddr;
}

__FIT_DECLSPEC
gaddr_type EvManager::reflect( addr_type addr ) const
{
  lock_guard<mutex> lk( _lock_xheap );
  ext_uuid_heap_type::const_iterator i = _ex_heap.find( addr );
  if ( i != _ex_heap.end() ) {
    return i->second;
  }
  return gaddr_type();
}

__FIT_DECLSPEC
void EvManager::Remove( void *channel )
{
  lock_guard<mutex> _x1( _lock_xheap );
  lock_guard<mutex> _x2( _lock_iheap );
  unsafe_Remove( channel );
}

void EvManager::settrf( unsigned f )
{
  lock_guard<mutex> _x1( _lock_tr );
  _trflags |= f;
}

void EvManager::unsettrf( unsigned f )
{
  lock_guard<mutex> _x1( _lock_tr );
  _trflags &= (0xffffffff & ~f);
}

void EvManager::resettrf( unsigned f )
{
  lock_guard<mutex> _x1( _lock_tr );
  _trflags = f;
}

void EvManager::cleantrf()
{
  lock_guard<mutex> _x1( _lock_tr );
  _trflags = 0;
}

unsigned EvManager::trflags() const
{
  lock_guard<mutex> _x1( _lock_tr );

  return _trflags;
}

void EvManager::settrs( std::ostream *s )
{
  lock_guard<mutex> _x1( _lock_tr );
  _trs = s;
}

// Remove references to remote objects, that was announced via 'channel'
// (related, may be, with socket connection)
// from [remote name -> local name] mapping table, and mark related session as
// 'disconnected'.
__FIT_DECLSPEC
void EvManager::unsafe_Remove( void *channel )
{
  pair<tr_uuid_heap_type::iterator,tr_uuid_heap_type::iterator> ch_range = _ch_heap.equal_range( channel );
  for (tr_uuid_heap_type::iterator i = ch_range.first; i != ch_range.second; ++i ) {
    pair<uuid_tr_heap_type::iterator,uuid_tr_heap_type::iterator> range = _tr_heap.equal_range( i->second );
    for ( uuid_tr_heap_type::iterator j = range.first; j != range.second; ) {
      if ( j->second.second.link == channel ) {
        _ex_heap.erase( j->second.first );
        iheap.erase( j->second.first );
        _tr_heap.erase( j++ );
        continue;
      }
      ++j;
    }
  }
  _ch_heap.erase( ch_range.first, ch_range.second );
}

__FIT_DECLSPEC const detail::transport& EvManager::transport( addr_type id ) const
{
  lock_guard<mutex> _x1( _lock_xheap );
  if ( (id & extbit) != 0 ) {
    ext_uuid_heap_type::const_iterator i = _ex_heap.find( id );
    if ( i == _ex_heap.end() ) {
      throw range_error( string( "no such address" ) );
    }
    pair<uuid_tr_heap_type::const_iterator,uuid_tr_heap_type::const_iterator> range = _tr_heap.equal_range( i->second );
    if ( range.first == _tr_heap.end() ) {
      throw range_error( string( "no transport" ) );
    }
    return min_element( range.first, range.second, tr_compare )->second.second;
  }
  throw range_error( string( "internal address" ) );
}

__FIT_DECLSPEC void EvManager::push( const Event& e )
{
  std::tr2::lock_guard<std::tr2::mutex> lk( _lock_queue );
  in_ev_queue.push_back( e );
  _cnd_queue.notify_one();
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
      ios_base::fmtflags f = _trs->flags( ios_base::hex | ios_base::showbase );
      *_trs << "EvManager push event " << e.code() << " to queue" << endl;
      _trs->flags( f );
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE
}

// Resolve Address -> Object Reference, call Object's dispatcher in case
// of local object, or call appropriate channel delivery function for
// remote object. All outgoing events, and incoming remote events
// (this method allow to forward event from remote object to another remote object,
// i.e. work as 'proxy' with 'transit objects')

void EvManager::Send( const Event& e )
{
  if ( e.dest() & extbit ) { // external object
    try {
      lock_guard<mutex> lk( _lock_xheap );
      ext_uuid_heap_type::const_iterator i = _ex_heap.find( e.dest() );
      if ( i == _ex_heap.end() ) { // destination not found
        ostringstream s;
        s << "external address unknown: " << hex << e.dest() << " from "
          << e.src() << ", pid " << dec << std::tr2::getpid();
        throw invalid_argument( s.str() );
      }

      pair<uuid_tr_heap_type::const_iterator,uuid_tr_heap_type::const_iterator> range = _tr_heap.equal_range( i->second );
      if ( range.first == _tr_heap.end() ) {
        throw range_error( string( "no transport" ) );
      }
      const detail::transport& tr = min_element( range.first, range.second, tr_compare )->second.second;
      detail::transport::kind_type k = tr.kind;
      void *link = tr.link;
      gaddr_type gaddr_dst( i->second );
      gaddr_type gaddr_src;

      ext_uuid_heap_type::const_iterator j = _ex_heap.find( e.src() );
      if ( j == _ex_heap.end() ) {
        gaddr_type& _gaddr_src = _ex_heap[e.src()];
        _gaddr_src.hid = xmt::hostid();
        _gaddr_src.pid = std::tr2::getpid();
        _gaddr_src.addr = e.src(); // it may be as local as foreign; if e.src()
        // is foreign, the object is 'transit object'
        gaddr_src = _gaddr_src;
      } else {
        gaddr_src = j->second;
      }

      lock_guard<mutex> _x2( _lock_iheap );

      switch ( k ) {
        case detail::transport::socket_tcp:
          if ( !reinterpret_cast<NetTransport_base *>(link)->push( e, gaddr_dst, gaddr_src) ) {
#ifdef __FIT_STEM_TRACE
            try {
              lock_guard<mutex> lk(_lock_tr);
              if ( _trs != 0 && _trs->good() && (_trflags & tracenet) ) {
                *_trs << "Remove net channel " << link << endl;
              }
            }
            catch ( ... ) {
            }
#endif // __FIT_STEM_TRACE
            // if I detect bad connection during writing to net
            // (in the push), I remove this connetion related entries.
            // Unsafe variant allow avoid deadlock here.
            unsafe_Remove( link );
          }
          break;
        case detail::transport::unknown:
          break;
        default:
          break;
      }
    }
    catch ( std::logic_error& err ) {
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << " " << __FILE__ << ":" << __LINE__ << endl;
          dump( *_trs );
        }
      }
      catch ( ... ) {
      }
    }
    catch ( std::runtime_error& err ) {
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << " " << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
    }
    catch ( ... ) {
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << "Unknown, uncatched exception: " << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
    }
  } else { // local object
    try {
      _lock_heap.lock();
      local_heap_type::iterator i = heap.find( e.dest() );
      if ( i == heap.end() ) { // destination not found
        throw invalid_argument( string("address unknown") );
      }
      EventHandler *object = i->second; // target object
      _lock_heap.unlock();

      try {
#ifdef __FIT_STEM_TRACE
        try {
          lock_guard<mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
            *_trs << object->classtype().name()
                  << " (" << object << ")\n";
            object->DispatchTrace( e, *_trs );
            *_trs << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_STEM_TRACE
        object->Dispatch( e ); // call dispatcher
      }
      catch ( std::logic_error& err ) {
        try {
          lock_guard<mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
            *_trs << err.what() << "\n"
                  << object->classtype().name() << " (" << object << ")\n";
            object->DispatchTrace( e, *_trs );
            *_trs << endl;
          }
        }
        catch ( ... ) {
        }
      }
      catch ( ... ) {
        try {
          lock_guard<mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
            *_trs << "Unknown, uncatched exception during process:\n"
                  << object->classtype().name() << " (" << object << ")\n";
            object->DispatchTrace( e, *_trs );
            *_trs << endl;
          }
        }
        catch ( ... ) {
        }
      }      
    }
    catch ( std::logic_error& err ) {
// #ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << "\n"
                << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
// #endif // __FIT_STEM_TRACE
      _lock_heap.unlock();
    }
    catch ( std::runtime_error& err ) {
// #ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << " "
                << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
// #endif // __FIT_STEM_TRACE
      _lock_heap.unlock();
    }
    catch ( ... ) {
// #ifdef __FIT_STEM_TRACE
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << "Unknown, uncatched exception: "
                << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
// #endif // __FIT_STEM_TRACE
      _lock_heap.unlock();
    }
  }
}

addr_type EvManager::create_unique()
{
  do {
    if ( ++_id > _high ) {
      _id = (_id - _low) % (_high - _low) + _low;
    }
  } while ( heap.find( _id ) != heap.end() );

  return _id;
}

addr_type EvManager::create_unique_x()
{
  do {
    if ( ++_x_id > _x_high ) {
      _x_id = (_x_id - _x_low) % (_x_high - _x_low) + _x_low;
    }
  } while ( _ex_heap.find( _x_id ) != _ex_heap.end() );

  return _x_id;
}

__FIT_DECLSPEC std::ostream& EvManager::dump( std::ostream& s ) const
{
  ios_base::fmtflags f = s.flags( ios_base::hex | ios_base::showbase );
  s << "Local map:\n";

  // s << hex << showbase;
  {
    lock_guard<mutex> lk( _lock_heap );

    for ( local_heap_type::const_iterator i = heap.begin(); i != heap.end(); ++i ) {
      s << i->first << "\t=> " << i->second << "\n";
    }
  }

  s << "\nInfo map:\n";
  {
    lock_guard<mutex> lk( _lock_iheap );

    for ( info_heap_type::const_iterator i = iheap.begin(); i != iheap.end(); ++i ) {
      s << i->first << "\t=> '" << i->second << "'\n";
    }
  }

  {
    lock_guard<mutex> lk( _lock_xheap );

    s << "\nExternal address map:\n";
    for ( ext_uuid_heap_type::const_iterator i = _ex_heap.begin(); i != _ex_heap.end(); ++i ) {
      s << hex << showbase << i->first << "\t=> " << i->second << "\n";
    }

    s << "\nUnique Id to transport map:\n";
    for ( uuid_tr_heap_type::const_iterator i = _tr_heap.begin(); i != _tr_heap.end(); ++i ) {
      s << i->first << "\t=> " << hex << i->second.first << " " << i->second.second.link << " " << dec << i->second.second.metric << "\n";
    }

    s << "\nTransport to Unique Id map:\n";
    for ( tr_uuid_heap_type::const_iterator i = _ch_heap.begin(); i != _ch_heap.end(); ++i ) {
      s << i->first << "\t=> " << i->second << "\n";
    }
  }

  s << endl;
  s.flags( f );

  return s;
}

} // namespace stem

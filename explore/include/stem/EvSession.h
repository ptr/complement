// -*- C++ -*- Time-stamp: <99/08/23 11:45:27 ptr>
#ifndef __EvSession_h
#define __EvSession_h

#ident "$SunId$ %Q%"

#include <string>
#include <ctime>
#include <sockstream>
#include <map>
#include <xmt.h>
#include <Event.h>

namespace EDS {

#ifdef __unix
using std::time;
#endif

struct SessionInfo
{
    SessionInfo() :
//        _start( 0 ),
//        _last( 0 ),
//        _conn( 0 ),
        _on_line( 0 ),
        _sz_from( 0 ),
        _sz_to( 0 ),
        _un_from( 0 ),
        _un_to( 0 ),
        _lun_from( 0 ),
        _lun_to( 0 ),
        _reconnect_cnt( 0 ),
        _control( Event::badaddr )
//        _is_connected( true )
      {
        _start = connect();
//        _start = time( &_last );
//        _conn = _last;
      }

//    ConnectSession( sockstream *s )
//      { connect( s ); }

    ~SessionInfo()
      { }

    // key_type key;

    // key_type ? secret_key;

    // machine identification:

//    union {
//	sockaddr_in inet;
//	sockaddr    any;
//    } _address;

    std::string _host; // name + ip +? port? 
    int _port;    // ?

    // sockstream *_sock; // ?

    // protocols stack:
//    sock_base::stype    _proto_low;     // TCP, UDP, etc
//    sock_base::protocol _proto_domain;  // inet/local
    // ????? proto_high; // row, over HTTP, etc.

    // billing info
    size_t   _sz_from;  // bytes transfer from
    size_t   _un_from;  // transfer units (say, events) from
    size_t   _sz_to;    // bytes transfer to
    size_t   _un_to;    // transfer units (say, events) to
    unsigned _lun_from; // last unit number had received
    unsigned _lun_to;   // last unit number sended;
    time_t   _start;    // session start time
    time_t   _on_line;  // time on line.
    time_t   _conn;     // last time of connect
    time_t   _last;     // last access time
    unsigned _reconnect_cnt; // reconnection counter
    bool     _is_connected;  // true if on line now (for stream connection)

    Event::key_type _control; // address of control object;

    // encoding / crypt info
    // ...


    bool is_connected() const
      { return _is_connected; }

    void inc_from( size_t sz, size_t u = 1 )
      {
        _sz_from += sz;
        _un_from += u;
        _last = time( 0 );
      }

    void inc_to( size_t sz, size_t u = 1 )
      {
        _sz_to += sz;
        _un_to += u;
      }

    unsigned un_from( unsigned u )
      {
        if ( ++_lun_from == u ) {
          return 0;
        }
        unsigned tmp = _lun_from;
        _lun_from = u;
        return u - tmp;
      }

    unsigned un_to( unsigned u )
      {
        if ( ++_lun_to == u ) {
          return 0;
        }
        unsigned tmp = _lun_to;
        _lun_to = u;
        return u - tmp;
      }

    time_t connect()
      {
        _conn = time( &_last );
        _is_connected = true;
        return _last;
      }

    time_t disconnect()
      {
        if ( _is_connected ) {
          _on_line += time( 0 ) - _conn;
          ++_reconnect_cnt;
          _is_connected = false;
        }
        return _on_line;
      }
};

template <class T>
class SessionManager
{
  public:
    typedef unsigned key_type;
//    typedef std::map<key_type,T> heap_type;
    typedef std::map<key_type,T,std::less<key_type>,
                __STL_DEFAULT_ALLOCATOR(T) > heap_type;

    SessionManager()
      { }

    key_type create()
      {
        MT_REENTRANT( _lock, _1 );
        key_type new_key = create_unique();
        heap[new_key];
        return new_key;
      }

    T& operator[]( const key_type& k )
      {
        MT_REENTRANT( _lock, _1 );
        return heap[k];
      }

    bool is_avail( const key_type& k ) const
      {
        MT_REENTRANT( _lock, _1 );
        return heap.find( k ) != heap.end();
      }

    void erase( const key_type& k )
      {
        MT_REENTRANT( _lock, _1 );
        heap.erase( k );
      }

    void erase( heap_type::iterator& k )
      {
        MT_REENTRANT( _lock, _1 );
        heap.erase( k );
      }

  protected:
    heap_type heap;

  private:
    key_type create_unique();
    static const key_type _low;
    static const key_type _high;
    static key_type _id;
    mutable typename __impl::Mutex _lock;
};

#ifndef _MSC_VER
template <class T>
const typename SessionManager<T>::key_type SessionManager<T>::_low( 0 );
#else
template <class T>
const SessionManager<T>::key_type SessionManager<T>::_low = 0;
#endif

#ifndef _MSC_VER
template <class T>
const typename SessionManager<T>::key_type SessionManager<T>::_high( 65535 );
#else
template <class T>
const SessionManager<T>::key_type SessionManager<T>::_high = 65535;
#endif

#ifndef _MSC_VER
template <class T>
typename SessionManager<T>::key_type SessionManager<T>::_id( SessionManager<T>::_low );
#else
template <class T>
SessionManager<T>::key_type SessionManager<T>::_id = SessionManager<T>::_low;
#endif

template <class T>
typename SessionManager<T>::key_type SessionManager<T>::create_unique()
{
#ifndef _MSC_VER
  std::pair<typename heap_type::iterator, bool> ret;
#else
  std::pair<heap_type::iterator,bool> ret;
#endif

  do {
    if ( ++_id > _high ) {
      _id = (_id - _low) % (_high - _low) + _low;
    }
  } while ( heap.find( _id ) != heap.end() );

  return _id;
}

typedef SessionManager<SessionInfo> EvSessionManager;

} // namespace EDS

#ifdef _MSC_VER
typedef EDS::SessionInfo SessionInfo;
#endif

#endif // __EvSession_h

// -*- C++ -*- Time-stamp: <2011-08-29 22:36:38 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002-2003, 2005-2011
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <config/feature.h>
#include <iterator>
#include <iomanip>
#include <sstream>
#include "stem/NetTransport.h"
#include "stem/EventHandler.h"
#include "stem/EvManager.h"
#include "crc.h"
#include "stem/EDSEv.h"
#include <mt/thread>
#include <mt/mutex>

#include <exam/defs.h>

#if !defined(STLPORT) && defined(__GNUC__) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
// for copy_n
# include <ext/algorithm>
#endif

const uint32_t EDS_MSG_LIMIT = 0x400000; // 4MB

namespace stem {

using namespace std;
using namespace std::tr2;

#if !defined(STLPORT) && defined(__GNUC__) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
using __gnu_cxx::copy_n;
#endif

#ifdef _BIG_ENDIAN
static const uint32_t EDS_MAGIC = 0xc2454453U;
#elif defined(_LITTLE_ENDIAN)
static const uint32_t EDS_MAGIC = 0x534445c2U;
#else
#  error "Can't determine platform byte order!"
#endif

extern mutex _def_lock;
extern addr_type _default_addr;

namespace detail {

// static const uint32_t net_transport_magic = 0xaaaa5555;

} // namespace detail

void dump( std::ostream& o, const EDS::Event& e )
{
  o << setiosflags(ios_base::showbase) << hex
    << "Code: " << e.code() << " Destination: " << e.dest().first << '/' << e.dest().second << " Source: " << e.src().first << '/' << e.src().second
    << "\nData:\n";

  string mark_line( "-----------| 4 --------| 8 --------| b --------|10 --------|14 --------|\n" );
  const int ln = 71;
  char buf[ln + 1];

  unsigned char c;
  string::const_iterator b = e.value().begin();

  int i = 0;
  int n = e.value().length();
  o << mark_line;
  o << resetiosflags(ios_base::showbase);
  while ( n-- ) {
    if ( i == ln ) {
      o << endl;
      buf[ln] = 0;
      o << buf << endl;
      i = 0;
    } else if ( i != 0 ) {
      o << " ";
      buf[i++] = ' ';
    }
    c =  static_cast<unsigned char>(*(b++));
    o << hex << setw(2) << setfill( '0' ) << unsigned(c);
    
    buf[i++] = ' ';
    buf[i++] = c > 0x1f && c < 0xff ? static_cast<char>(c) : ' ';
  }
  if ( i != 0 ) {
    o << endl;
    buf[i] = 0;
    o << buf << endl;   
  }
}

#if 0
addr_type NetTransport_base::ns_remote() const
{
  if ( net.is_open() ) {
    /* self, born address
       see NetTransport::discovery and NetTransport::connect
       about assign of peer's NS */
    return _id;
  }

  return badaddr;
}

void NetTransport_base::add_route( const addr_type& a )
{
  std::tr2::lock_guard<std::tr2::recursive_mutex> lk( this->_theHistory_lock );
  _ids.push_back( a );
  manager()->Subscribe( a, this, 1000 );
}

void NetTransport_base::rm_route( const addr_type& a )
{
  std::tr2::lock_guard<std::tr2::recursive_mutex> lk( this->_theHistory_lock );
  addr_container_type::iterator i = _ids.begin();
  ++i;
  while ( i != _ids.end() ) {
    if ( *i == a ) {
      _ids.erase( i++ );
      manager()->Unsubscribe( a, this );
    } else {
      ++i;
    }
  }
}

void NetTransport_base::add_remote_route( const addr_type& addr )
{
  Event ev( EV_STEM_ANNOTATION );

  ev.dest( badaddr ); // special: no destination
  ev.src( addr );     // will be annotated in NetTransport::connect
                      // or NetTransportMgr::_loop

  NetTransport_base::Dispatch( ev );
}

void NetTransport_base::rm_remote_route( const addr_type& addr )
{
}

#endif

__FIT_DECLSPEC void NetTransport_base::_close()
{
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
    if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & (EvManager::tracenet)) ) {
      ios_base::fmtflags f = EventHandler::manager()._trs->flags( ios_base::showbase );
      *EventHandler::manager()._trs << "NetTransport_base::_close " << _id << endl;
#ifdef STLPORT
      EventHandler::manager()._trs->flags( f );
#else
      EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

#if 0
  EventHandler::solitary();
#endif
  net.close();

  if ( _id != xmt::nil_uuid ) {
    // EventHandler::manager().remove_edge( _id );
    _id = xmt::nil_uuid;
  }
}

void NetTransport_base::close()
{
  if ( _id != xmt::nil_uuid ) {
    EventHandler::manager().remove_edge( _id );
  }
  NetTransport_base::_close();
}


bool NetTransport_base::pop( Event& _rs )
{
  msg_hdr header;

  using namespace std;

  if ( !net.read( (char *)&header.magic, sizeof(uint32_t) ).good() ) {
    net.setstate( ios_base::failbit );
    return false;
  }

  if ( header.magic != EDS_MAGIC ) {
    try {
      lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
      if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *EventHandler::manager()._trs << HERE << " StEM Magic fail ("
                                         << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port() << ")"
                                         << endl;
#ifdef STLPORT
        EventHandler::manager()._trs->flags( f );
#else
        EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }

    net.setstate( ios_base::failbit );
    return false;
  }

  if ( !net.read( (char *)&header.code, sizeof(msg_hdr) - sizeof(uint32_t) ).good() ) {
    net.setstate( ios_base::failbit );
    return false;
  }

  _rs.code( from_net( header.code ) );
  domain_type dstd;
  dstd.u.i[0] = header.dstd[0];
  dstd.u.i[1] = header.dstd[1];
  dstd.u.i[2] = header.dstd[2];
  dstd.u.i[3] = header.dstd[3];

  domain_type srcd;
  srcd.u.i[0] = header.srcd[0];
  srcd.u.i[1] = header.srcd[1];
  srcd.u.i[2] = header.srcd[2];
  srcd.u.i[3] = header.srcd[3];

  addr_type dst;
  dst.u.i[0] = header.dst[0];
  dst.u.i[1] = header.dst[1];
  dst.u.i[2] = header.dst[2];
  dst.u.i[3] = header.dst[3];

  addr_type src;
  src.u.i[0] = header.src[0];
  src.u.i[1] = header.src[1];
  src.u.i[2] = header.src[2];
  src.u.i[3] = header.src[3];

  _rs.src( make_pair( srcd, src ) );
  _rs.dest( make_pair( dstd, dst ) );

  _rs.resetf( from_net( header.flags ) );
  uint32_t sz = from_net( header.sz );

  if ( sz >= EDS_MSG_LIMIT ) {
    try {
      lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
      if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *EventHandler::manager()._trs << HERE << " StEM Message size too big: " << sz
                                         << " (" << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port()
                                         << ")" << endl;
#ifdef STLPORT
        EventHandler::manager()._trs->flags( f );
#else
        EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
        }
    }
    catch ( ... ) {
    }
    net.setstate( ios_base::failbit );
    return false;
  }

  adler32_type adler = adler32( (unsigned char *)&header, sizeof(msg_hdr) - sizeof(uint32_t) );
  if ( adler != from_net( header.crc ) ) {
    try {
      lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
      if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *EventHandler::manager()._trs << HERE << " StEM CRC fail"
                                         << " (" << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port()
                                         << ")" << endl;
#ifdef STLPORT
        EventHandler::manager()._trs->flags( f );
#else
        EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
    net.setstate( ios_base::failbit );
    return false;
  }

  string& str = _rs.value();

  str.erase();
  str.reserve( sz );
  copy_n( istreambuf_iterator<char>(net), sz, back_inserter(str) );

#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
    if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & (EvManager::tracenet)) ) {
#ifdef STLPORT
      ios_base::fmtflags flags = EventHandler::manager()._trs->flags( 0 );
#else
      ios_base::fmtflags flags = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
      *EventHandler::manager()._trs << "\tMessage from remote " << hex << showbase << _rs.code() << " "
                                    << srcd << '/' << src
                                    << " -> "
                                    << dstd << '/' << dst << endl;
#ifdef STLPORT
      EventHandler::manager()._trs->flags( flags );
#else
      EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(flags) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  if ( !net.good() ) {
    net.setstate( ios_base::failbit );
  }

  return net.good();
}

bool NetTransport_base::Dispatch( const Event& _rs )
{
  //if ( _rs.dest() == _ids.front() ) {
    // to be useful, derive NetTransport_base from another class, derived from EventHandler
  //  return EventHandler::Dispatch( _rs );
  // }
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
    if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & (EvManager::tracenet)) ) {
#ifdef STLPORT
      ios_base::fmtflags flags = EventHandler::manager()._trs->flags( 0 );
#else
      ios_base::fmtflags flags = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
      *EventHandler::manager()._trs << "\tMessage to remote " << hex << showbase << _rs.code() << " "
                                    << _rs.src().first << '/' << _rs.src().second
                                    << " -> "
                                    << _rs.dest().first << '/' << _rs.dest().second << endl;
      EventHandler::manager().dump( *EventHandler::manager()._trs ) << endl;
#ifdef STLPORT
      EventHandler::manager()._trs->flags( flags );
#else
      EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(flags) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  if ( !net.good() ) {
    return false;
  }

  msg_hdr header;

  header.magic = EDS_MAGIC;
  header.code = to_net( _rs.code() );

  ext_addr_type dst = _rs.dest();
  ext_addr_type src = _rs.src();

  header.dstd[0] = dst.first.u.i[0];
  header.dstd[1] = dst.first.u.i[1];
  header.dstd[2] = dst.first.u.i[2];
  header.dstd[3] = dst.first.u.i[3];

  header.srcd[0] = src.first.u.i[0];
  header.srcd[1] = src.first.u.i[1];
  header.srcd[2] = src.first.u.i[2];
  header.srcd[3] = src.first.u.i[3];

  header.dst[0] = dst.second.u.i[0];
  header.dst[1] = dst.second.u.i[1];
  header.dst[2] = dst.second.u.i[2];
  header.dst[3] = dst.second.u.i[3];

  header.src[0] = src.second.u.i[0];
  header.src[1] = src.second.u.i[1];
  header.src[2] = src.second.u.i[2];
  header.src[3] = src.second.u.i[3];

  header.flags = to_net( _rs.flags() );
  header.sz = to_net( static_cast<uint32_t>(_rs.value().size()) );
  header.crc = to_net( adler32( (unsigned char *)&header, sizeof(msg_hdr) - sizeof(uint32_t) )); // crc

  try {
    net.write( (const char *)&header, sizeof(msg_hdr) );
    if ( !_rs.value().empty() ) {
      net.write( _rs.value().data(), _rs.value().size() );
    }

    net.flush();
  }
  catch ( ios_base::failure& ) {
  }
  catch ( ... ) {
    throw;
  }

  return net.good();
}

domain_type NetTransport_base::domain() const
{
  if ( _id == xmt::nil_uuid ) {
    return xmt::nil_uuid;
  }

  basic_read_lock<rw_mutex> lk( EventHandler::manager()._lock_edges );

  auto eid =  EventHandler::manager().edges.find( _id );
 
  if ( eid == EventHandler::manager().edges.end() ) {
    return xmt::nil_uuid;
  }

  return eid->second.first.second; // peer's domain
}

int NetTransport_base::flags() const
{
  return EvManager::remote;
}

NetTransport::NetTransport( std::sockstream& s ) :
    NetTransport_base( s ),
    exchange( false )
{
  try {
    if ( s.rdbuf()->family() == AF_INET) {
      s.rdbuf()->setoptions( sock_base::so_tcp_nodelay );
    }

    s.write( reinterpret_cast<const char*>(&EDS_MAGIC), sizeof(EDS_MAGIC) );
    s.flush();
  }
  catch ( ... ) {
  }
}

NetTransport::~NetTransport()
{
  if ( _id != xmt::nil_uuid ) {
    EventHandler::manager().remove_edge( _id );
  }
  NetTransport_base::_close();
}

void NetTransport::connect( sockstream& s )
{
  try {
    if ( !exchange ) {
      uint32_t magic;

      s.read( reinterpret_cast<char*>(&magic), sizeof(EDS_MAGIC) );

      if ( s.fail() || magic != EDS_MAGIC ) {
        throw ios_base::failure( "EDS_MAGIC fail" );
      }

      domain_type domain;

      __pack_base::__unpack( s, domain );

      if ( s.fail() || (domain == xmt::nil_uuid) ) {
        throw ios_base::failure( "bad peer's domain" );
      }

      EvManager::edge_id_type eid;
      domain_type u;
      domain_type v;
      uint32_t w;

      for ( ; ; ) {
        __pack_base::__unpack( s, eid );
        if ( s.fail() ) {
          throw ios_base::failure( "domains exchange fail" );
        } else if ( eid == xmt::nil_uuid ) {
          break; // finish of edges list
        }
        __pack_base::__unpack( s, u );
        __pack_base::__unpack( s, v );
        __pack_base::__unpack( s, w );

        if ( s.fail() ) {
          throw ios_base::failure( "domains exchange fail" );
        }

        EventHandler::manager().connectivity( eid, u, v, w, 0 );
      }


      // edges, that come from peer will be returned too,
      // but it ok; __unpack (above) should be before __pack (below),
      // otherwise stalling possible (if my graph or graph from peer
      // exceeds n*MTU---see sockstream's buffer).

      _id = EventHandler::manager().bridge( this, domain );

      EvManager::edge_id_type other_eid = xmt::uid(); // create it for peer
      EventHandler::manager().connectivity( other_eid, domain, EventHandler::domain(), 1000, 0 );
      __pack_base::__pack( s, other_eid );
      __pack_base::__pack( s, domain );
      __pack_base::__pack( s, EventHandler::domain() );
      __pack_base::__pack( s, 1000 );

      basic_read_lock<rw_mutex> lk( EventHandler::manager()._lock_edges );

      for ( auto i = EventHandler::manager().edges.begin(); i != EventHandler::manager().edges.end(); ++i ) {
        if ( i->first != other_eid ) {
          __pack_base::__pack( s, i->first );
          __pack_base::__pack( s, i->second.first.first );
          __pack_base::__pack( s, i->second.first.second );
          __pack_base::__pack( s, static_cast<uint32_t>(i->second.second) );
        }
      }
      __pack_base::__pack( s, xmt::nil_uuid );
      s.flush();

      EventHandler::manager().route_calc();

      exchange = true;

      return;
    }

    Event ev;

    if ( pop( ev ) ) {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
        if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & EvManager::tracenet) ) {
#ifdef STLPORT
          ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
          ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
          *EventHandler::manager()._trs << "Pid/ppid: " << std::tr2::getpid() << "/" << std::tr2::getppid() << "\n";
          EventHandler::manager().dump( *EventHandler::manager()._trs ) << endl;
#ifdef STLPORT
          EventHandler::manager()._trs->flags( f );
#else
          EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>( f ) );
#endif
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE

      EventHandler::manager().push( ev );
    } else {
      if ( _id != xmt::nil_uuid ) {
        EventHandler::manager().remove_edge( _id );
      }
      NetTransport_base::_close();
    }
  }
  catch ( ios_base::failure& ex ) {
    s.close();
    try {
      lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
      if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *EventHandler::manager()._trs << HERE << " " << ex.what()
                         << " (" << s.rdbuf()->inet_addr() << ":" << s.rdbuf()->port()
                         << ")" << endl;
#ifdef STLPORT
        EventHandler::manager()._trs->flags( f );
#else
        EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
    try {
      lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
      if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *EventHandler::manager()._trs << __FILE__ << ":" << __LINE__ << " unknown exception"
                         << " (" << s.rdbuf()->inet_addr() << ":" << s.rdbuf()->port()
                         << ")" << endl;
#ifdef STLPORT
        EventHandler::manager()._trs->flags( f );
#else
        EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }

    s.close();
  }
}

addr_type NetTransportMgr::open( const char* hostname, int port,
                                 std::sock_base::stype stype,
                                 sock_base::protocol pro )
{
  std::sockstream::open( hostname, port, stype, pro );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    try {
      if ( (pro == sock_base::inet) && (stype == sock_base::sock_stream) ) {
        std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
      }
    }
    catch ( ... ) {
    }
    return discovery();
  }
  return badaddr;
}

addr_type NetTransportMgr::open( const char* path,
                                 std::sock_base::stype stype )
{
  std::sockstream::open( path, stype );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    return discovery();
  }
  return badaddr;
}

addr_type NetTransportMgr::open( in_addr_t addr, int port,
                                 sock_base::stype type,
                                 sock_base::protocol pro )
{
  std::sockstream::open( addr, port, type, pro );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    try {
      if ( (pro == sock_base::inet) && (type == sock_base::sock_stream) ) {
        std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
      }
    }
    catch ( ... ) {
    }
    return discovery();
  }
  return badaddr;
}

addr_type NetTransportMgr::open( const sockaddr_in& addr,
                                 sock_base::stype type )
{
  std::sockstream::open( addr, type );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    try {
      if ( type == sock_base::sock_stream ) {
        std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
      }
    }
    catch ( ... ) {
    }
    return discovery();
  }
  return badaddr;
}

addr_type NetTransportMgr::open( sock_base::socket_type s, const sockaddr& addr,
                                 sock_base::stype type )
{
  std::sockstream::open( s, addr, type );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    try {
      if ( (addr.sa_family == AF_INET) && (type == sock_base::sock_stream) ) {
        std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
      }
    }
    catch ( ... ) {
    }
    return discovery();
  }
  return badaddr;
}

addr_type NetTransportMgr::open( sock_base::socket_type s,
                                 sock_base::stype type )
{
  std::sockstream::open( s, type );
  if ( std::sockstream::is_open() && std::sockstream::good() ) {
    try {
      // std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
    }
    catch ( ... ) {
    }
    return discovery();
  }
  return badaddr;
}

void NetTransportMgr::close()
{
  // stem::EvManager::edge_id_type tmp = _id;

  if ( _id /* tmp */ != xmt::nil_uuid ) {
    EventHandler::manager().remove_edge( _id );
  }
  NetTransport_base::_close();
}

stem::addr_type NetTransportMgr::discovery( const std::tr2::nanoseconds& timeout )
{
  this->write( reinterpret_cast<const char*>(&EDS_MAGIC), sizeof(EDS_MAGIC) );
  __pack_base::__pack( *this, EventHandler::domain() );
  this->flush();
  
  if ( timeout != nanoseconds() ) {
    std::sockstream::rdbuf()->rdtimeout( timeout );
  }

  uint32_t magic;

  this->read( reinterpret_cast<char*>(&magic), sizeof(EDS_MAGIC) );

  if ( this->fail() || magic != EDS_MAGIC ) {
    return stem::badaddr;
  }

  {
    basic_read_lock<rw_mutex> lk( EventHandler::manager()._lock_edges );

    for ( auto i = EventHandler::manager().edges.begin(); i != EventHandler::manager().edges.end(); ++i ) {
      __pack_base::__pack( *this, i->first );
      __pack_base::__pack( *this, i->second.first.first );
      __pack_base::__pack( *this, i->second.first.second );
      __pack_base::__pack( *this, static_cast<uint32_t>(i->second.second) );
    }
  }
  __pack_base::__pack( *this, xmt::nil_uuid );
  this->flush();

  domain_type domain;
  EvManager::edge_id_type eid;
  domain_type u;
  domain_type v;
  uint32_t w;

  bool first = true;

  for ( ; ; ) {
    __pack_base::__unpack( *this, eid );
    if ( this->fail()  ) {
      return stem::badaddr;
    }
    if ( eid == xmt::nil_uuid ) {
      break; // finish of edges list
    }
    __pack_base::__unpack( *this, u );
    __pack_base::__unpack( *this, v );
    __pack_base::__unpack( *this, w );

    if ( this->fail() ) {
      return stem::badaddr;
    }

    if ( first ) {
      EventHandler::manager().connectivity( eid, u, v, w, this );
      domain = v; // u == EventHandler::manager()->_id;
      _id = eid;
      first = false;
    } else {
      EventHandler::manager().connectivity( eid, u, v, w, 0 );
    }
  }  

  if ( !this->fail() && (domain != xmt::nil_uuid) ) {
    EventHandler::manager().route_calc();
    _thr = new std::tr2::thread( _loop, this );
    return domain;
  }

  return stem::badaddr;
}


void NetTransportMgr::_loop( NetTransportMgr* p )
{
  NetTransportMgr& me = *p;
  Event ev;

  try {
    me.rdbuf()->rdtimeout(); // reset not to use rdtimeout 
    while ( me.pop( ev ) ) {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
        if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & EvManager::tracenet) ) {
#ifdef STLPORT
          ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
          ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
          *EventHandler::manager()._trs << "Pid/ppid: " << std::tr2::getpid() << "/" << std::tr2::getppid() << "\n";
          EventHandler::manager().dump( *EventHandler::manager()._trs ) << endl;
          *EventHandler::manager()._trs << "NetTransportMgr " << me._id << " accept event "
                           << hex << showbase << ev.code() << endl;
#ifdef STLPORT
          EventHandler::manager()._trs->flags( f );
#else
          EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE

      EventHandler::manager().push( ev );
    }
    me.net.close();
    if ( me._id != xmt::nil_uuid ) {
      EventHandler::manager().remove_edge( me._id );
    }
    me.NetTransport_base::_close();
#ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
      if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & (EvManager::tracenet)) ) {
#ifdef STLPORT
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *EventHandler::manager()._trs << "NetTransportMgr " << me._id << " loop exit" << endl;
#ifdef STLPORT
        EventHandler::manager()._trs->flags( f );
#else
        EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_STEM_TRACE
  }
  catch ( ... ) {
    me.net.close();
    if ( me._id != xmt::nil_uuid ) {
      EventHandler::manager().remove_edge( me._id );
    }
    me.NetTransport_base::_close();
#ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(EventHandler::manager()._lock_tr);
      if ( EventHandler::manager()._trs != 0 && EventHandler::manager()._trs->good() && (EventHandler::manager()._trflags & (EvManager::tracenet)) ) {
#ifdef STLPORT
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( 0 );
#else
        ios_base::fmtflags f = EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *EventHandler::manager()._trs << "NetTransportMgr " << me._id << " loop exit due to exception" << endl;
#ifdef STLPORT
        EventHandler::manager()._trs->flags( f );
#else
        EventHandler::manager()._trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_STEM_TRACE
  }
}

void NetTransportMgr::join()
{
  if ( _thr != 0 && _thr->joinable() ) {
    _thr->join();
    delete _thr;
    _thr = 0;
  }
}

} // namespace stem

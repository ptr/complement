// -*- C++ -*- Time-stamp: <09/06/24 20:03:29 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002-2003, 2005-2009
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

const uint32_t EDS_MSG_LIMIT = 0x400000; // 4MB

namespace stem {

using namespace std;
using namespace std::tr2;

#ifdef _BIG_ENDIAN
static const uint32_t EDS_MAGIC = 0xc2454453U;
#elif defined(_LITTLE_ENDIAN)
static const uint32_t EDS_MAGIC = 0x534445c2U;
#else
#  error "Can't determine platform byte order!"
#endif

extern mutex _def_lock;
extern addr_type _default_addr;

__FIT_DECLSPEC
void dump( std::ostream& o, const EDS::Event& e )
{
  o << setiosflags(ios_base::showbase) << hex
    << "Code: " << e.code() << " Destination: " << e.dest() << " Source: " << e.src()
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

addr_type NetTransport_base::ns_remote() const
{
  if ( net.is_open() ) {
    id_iterator i = EventHandler::self_ids_begin();
    if ( i != EventHandler::self_ids_end() ) { // self, born address
      // see NetTransport::discovery and NetTransport::connect
      // about assign of peer's NS
      if ( ++i != EventHandler::self_ids_end() ) {
        return *i;
      }
    }
  }

  return badaddr;
}

__FIT_DECLSPEC void NetTransport_base::_close()
{
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(manager()->_lock_tr);
    if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
      ios_base::fmtflags f = manager()->_trs->flags( ios_base::showbase );
      *manager()->_trs << "NetTransport_base::_close " << self_id() << endl;
#ifdef STLPORT
      manager()->_trs->flags( f );
#else
      manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif

    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  net.close();
  EventHandler::solitary();
}

bool NetTransport_base::pop( Event& _rs )
{
  msg_hdr header;

  using namespace std;

  if ( !net.read( (char *)&header.magic, sizeof(uint32_t) ).good() ) {
    return false;
  }

  if ( header.magic != EDS_MAGIC ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
        ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *manager()->_trs << HERE << " StEM Magic fail ("
                         << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port() << ")"
                         << endl;
#ifdef STLPORT
        manager()->_trs->flags( f );
#else
        manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }

    net.close();
    return false;
  }

  if ( !net.read( (char *)&header.code, sizeof(msg_hdr) - sizeof(uint32_t) ).good() ) {
    net.close();
    return false;
  }
  _rs.code( from_net( header.code ) );
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

  _rs.src( src );
  _rs.dest( dst );

  uint32_t _x_count = from_net( header.pad3 );
  _rs.resetf( from_net( header.flags ) );
  uint32_t sz = from_net( header.sz );

  if ( sz >= EDS_MSG_LIMIT ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
        ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *manager()->_trs << HERE << " StEM Message size too big: " << sz
                         << " (" << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port()
                         << ")" << endl;
#ifdef STLPORT
        manager()->_trs->flags( f );
#else
        manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
    net.close();
    return false;
  }

  adler32_type adler = adler32( (unsigned char *)&header, sizeof(msg_hdr) - sizeof(uint32_t) );
  if ( adler != from_net( header.crc ) ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
        ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *manager()->_trs << HERE << " StEM CRC fail"
                         << " (" << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port()
                         << ")" << endl;
#ifdef STLPORT
        manager()->_trs->flags( f );
#else
        manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
    net.close();
    return false;
  }

  string& str = _rs.value();

  str.erase();
  str.resize( sz );
  net.read( const_cast<char*>(str.data()), sz );
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(manager()->_lock_tr);
    if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
#ifdef STLPORT
      ios_base::fmtflags flags = manager()->_trs->flags( 0 );
#else
      ios_base::fmtflags flags = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
      *manager()->_trs << "\tMessage from remote " << hex << showbase << _rs.code() << " "
                       << src << " -> " << dst << endl;
#ifdef STLPORT
      manager()->_trs->flags( flags );
#else
      manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(flags) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

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
    lock_guard<mutex> lk(manager()->_lock_tr);
    if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
#ifdef STLPORT
      ios_base::fmtflags flags = manager()->_trs->flags( 0 );
#else
      ios_base::fmtflags flags = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
      *manager()->_trs << "\tMessage to remote " << hex << showbase << _rs.code() << " "
                       << _rs.src() << " -> " << _rs.dest() << endl;
      manager()->dump( *manager()->_trs ) << endl;
#ifdef STLPORT
      manager()->_trs->flags( flags );
#else
      manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(flags) );
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

  fill( (char *)&header, (char *)&header + sizeof(msg_hdr), 0U );

  header.magic = EDS_MAGIC;
  header.code = to_net( _rs.code() );

  addr_type dst = _rs.dest();
  addr_type src = _rs.src();

  header.dst[0] = dst.u.i[0];
  header.dst[1] = dst.u.i[1];
  header.dst[2] = dst.u.i[2];
  header.dst[3] = dst.u.i[3];

  header.src[0] = src.u.i[0];
  header.src[1] = src.u.i[1];
  header.src[2] = src.u.i[2];
  header.src[3] = src.u.i[3];

  header.pad3 = to_net( ++_count );
  header.flags = to_net( _rs.flags() );
  header.sz = to_net( static_cast<uint32_t>(_rs.value().size()) );
  header.crc = to_net( adler32( (unsigned char *)&header, sizeof(msg_hdr) - sizeof(uint32_t) )); // crc

  try {
    net.write( (const char *)&header, sizeof(msg_hdr) );
    if ( !_rs.value().empty() ) {
      net.write( _rs.value().data(), _rs.value().size() );
    }

    // copy( _rs.value().begin(), _rs.value().end(),
    //      ostream_iterator<char,char,char_traits<char> >(net) );

    net.flush();
    if ( !net.good() ) {
      throw ios_base::failure( "net not good" );
    }
  }
  catch ( ios_base::failure& ) {
    net.close();
  }
  catch ( ... ) {
    throw;
  }

  return net.good();
}

__FIT_DECLSPEC
NetTransport::NetTransport( std::sockstream& s ) :
    NetTransport_base( s, "stem::NetTransport" ),
    exchange( false )
{
  try {
    s.rdbuf()->setoptions( sock_base::so_tcp_nodelay );

    __pack_base::__pack( s, EventHandler::ns() );
    {
      lock_guard<mutex> lk( _def_lock );
      __pack_base::__pack( s, _default_addr );
    }
    s.flush();
  }
  catch ( ... ) {
  }
}

__FIT_DECLSPEC
void NetTransport::connect( sockstream& s )
{
  try {
    if ( !exchange ) {
      addr_type ns;
      addr_type fdef;
      __pack_base::__unpack( s, ns );
      __pack_base::__unpack( s, fdef );

      _ids.push_back( ns );
      manager()->Subscribe( ns, this, "nsf", 1000 );
      if ( fdef != xmt::nil_uuid ) {
        _ids.push_back( fdef );
        manager()->Subscribe( fdef, this, "foreign default", 1000 );
      }

      exchange = true;
      return;
    }

    Event ev;

    if ( pop( ev ) ) {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(manager()->_lock_tr);
        if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracenet) ) {
#ifdef STLPORT
          ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
          ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
          *manager()->_trs << "Pid/ppid: " << std::tr2::getpid() << "/" << std::tr2::getppid() << "\n";
          manager()->dump( *manager()->_trs ) << endl;
#ifdef STLPORT
          manager()->_trs->flags( f );
#else
          manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>( f ) );
#endif
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE

      if ( !manager()->is_avail( ev.src() ) ) {
        _ids.push_back( ev.src() );
        manager()->Subscribe( ev.src(), this, 1000 );
      }

      manager()->push( ev );
    }
  }
  catch ( ios_base::failure& ex ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
        ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *manager()->_trs << HERE << " " << ex.what()
                         << " (" << s.rdbuf()->inet_addr() << ":" << s.rdbuf()->port()
                         << ")" << endl;
#ifdef STLPORT
        manager()->_trs->flags( f );
#else
        manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
#ifdef STLPORT
        ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
        ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " unknown exception"
                         << " (" << s.rdbuf()->inet_addr() << ":" << s.rdbuf()->port()
                         << ")" << endl;
#ifdef STLPORT
        manager()->_trs->flags( f );
#else
        manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }

    s.close();
  }
}

// connect initiator (client) function

addr_type NetTransportMgr::open( const char *hostname, int port,
                                 std::sock_base::stype stype,
                                 sock_base::protocol pro )
{
  std::sockstream::open( hostname, port, stype, pro );
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
      std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
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
      std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
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
      std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
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
      std::sockstream::rdbuf()->setoptions( sock_base::so_tcp_nodelay );
    }
    catch ( ... ) {
    }
    return discovery();
  }
  return badaddr;
}

addr_type NetTransportMgr::discovery()
{
  __pack_base::__pack( *this, EventHandler::ns() );
  {
    lock_guard<mutex> lk( _def_lock );
    __pack_base::__pack( *this, _default_addr );
  }
  this->flush();
  
  addr_type ns;
  addr_type fdef;
  __pack_base::__unpack( *this, ns );
  __pack_base::__unpack( *this, fdef );

  if ( !this->fail() ) {
    _ids.push_back( ns );
    manager()->Subscribe( ns, this, "nsf", 1000 );
    if ( fdef != xmt::nil_uuid ) {
      _ids.push_back( fdef );
      manager()->Subscribe( fdef, this, "foreign default", 1000 );
    }
    _thr = new std::tr2::thread( _loop, this );
    return fdef;
    // return ns;
  }

  return badaddr;
}

void NetTransportMgr::_loop( NetTransportMgr* p )
{
  NetTransportMgr& me = *p;
  Event ev;

  addr_type self = me.self_id();

  try {
    while ( me.pop( ev ) ) {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(manager()->_lock_tr);
        if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracenet) ) {
#ifdef STLPORT
          ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
          ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
          *manager()->_trs << "Pid/ppid: " << std::tr2::getpid() << "/" << std::tr2::getppid() << "\n";
          manager()->dump( *manager()->_trs ) << endl;
          *manager()->_trs << "NetTransportMgr " << self << " accept event "
                           << hex << showbase << ev.code() << endl;
#ifdef STLPORT
          manager()->_trs->flags( f );
#else
          manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE

      if ( !manager()->is_avail( ev.src() ) ) {
        me._ids.push_back( ev.src() );
        manager()->Subscribe( ev.src(), &me, 1000 );
      }

      manager()->push( ev );
    }
    me.net.close();
#ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
#ifdef STLPORT
        ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
        ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *manager()->_trs << "NetTransportMgr " << self << " loop exit" << endl;
#ifdef STLPORT
        manager()->_trs->flags( f );
#else
        manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_STEM_TRACE
  }
  catch ( ... ) {
    me.net.close();
#ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
#ifdef STLPORT
        ios_base::fmtflags f = manager()->_trs->flags( 0 );
#else
        ios_base::fmtflags f = manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(0) );
#endif
        *manager()->_trs << "NetTransportMgr " << me.self_id() << " loop exit due to exception" << endl;
#ifdef STLPORT
        manager()->_trs->flags( f );
#else
        manager()->_trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_STEM_TRACE
    // throw;
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

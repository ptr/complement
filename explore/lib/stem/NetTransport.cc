// -*- C++ -*- Time-stamp: <09/04/30 10:57:40 ptr>

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

__FIT_DECLSPEC void NetTransport_base::_close()
{
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(manager()->_lock_tr);
    if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
      ios_base::fmtflags f = manager()->_trs->flags( ios_base::hex | ios_base::showbase );
      *manager()->_trs << "NetTransport_base::_close " << self_id() << endl;
      manager()->_trs->flags( f );
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_STEM_TRACE

  EventHandler::addr_container_type::iterator j = _ids.begin();
  ++j;
  for ( ; j != _ids.end(); ++j ) {    
    manager()->Unsubscribe( *j );
  }
  net.close();
}

bool NetTransport_base::pop( Event& _rs )
{
  const int bsz = 2+(4+2+1)*2+4;
  uint32_t buf[bsz];
  using namespace std;

  if ( !net.read( (char *)buf, sizeof(uint32_t) ).good() ) {
    return false;
  }

  if ( buf[0] != EDS_MAGIC ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " StEM Magic fail ("
                         << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port() << ")"
                         << endl;
      }
    }
    catch ( ... ) {
    }

    _close();
    return false;
  }

  if ( !net.read( (char *)&buf[1], sizeof(uint32_t) * (bsz-1) ).good() ) {
    _close();
    return false;
  }
  _rs.code( from_net( buf[1] ) );
  addr_type dst;
  dst.u.i[0] = buf[2];
  dst.u.i[1] = buf[3];
  dst.u.i[2] = buf[4];
  dst.u.i[3] = buf[5];

  addr_type src;
  src.u.i[0] = buf[9];
  src.u.i[1] = buf[10];
  src.u.i[2] = buf[11];
  src.u.i[3] = buf[12];

  _rs.src( src );
  _rs.dest( dst );

  uint32_t _x_count = from_net( buf[16] );
  _rs.resetf( from_net( buf[17] ) );
  uint32_t sz = from_net( buf[18] );

  if ( sz >= EDS_MSG_LIMIT ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " StEM Message size too big: " << sz
                         << " (" << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port()
                         << ")" << endl;
      }
    }
    catch ( ... ) {
    }
    _close();
    return false;
  }

  adler32_type adler = adler32( (unsigned char *)buf, sizeof(uint32_t) * 19 );
  if ( adler != from_net( buf[19] ) ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " StEM Adler-32 fail"
                         << " (" << net.rdbuf()->inet_addr() << ":" << net.rdbuf()->port()
                         << ")" << endl;
      }
    }
    catch ( ... ) {
    }

    _close();
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
      int flags = manager()->_trs->flags();
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
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(manager()->_lock_tr);
    if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
      int flags = manager()->_trs->flags();
      *manager()->_trs << "\tMessage to remote " << hex << showbase << _rs.code() << " "
                       << _rs.src() << " -> " << _rs.dest() << endl;
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
  const int bsz = 2+(4+2+1)*2+4;
  uint32_t buf[bsz];

  fill( buf, buf + bsz, 0U ); // Hmm, here problem?

  buf[0] = EDS_MAGIC;
  buf[1] = to_net( _rs.code() );

  addr_type dst = _rs.dest();
  addr_type src = _rs.src();

  buf[2] = dst.u.i[0];
  buf[3] = dst.u.i[1];
  buf[4] = dst.u.i[2];
  buf[5] = dst.u.i[3];

  buf[9] = src.u.i[0];
  buf[10] = src.u.i[1];
  buf[11] = src.u.i[2];
  buf[12] = src.u.i[3];

  buf[16] = to_net( ++_count );
  buf[17] = to_net( _rs.flags() );
  buf[18] = to_net( static_cast<uint32_t>(_rs.value().size()) );
  buf[19] = to_net( adler32( (unsigned char *)buf, sizeof(uint32_t) * 19 )); // crc

  try {
    net.write( (const char *)buf, sizeof(uint32_t) * 20 );
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
    NetTransport_base( s, "stem::NetTransport" )
{
  try {
    s.rdbuf()->setoptions( sock_base::so_tcp_nodelay );
  }
  catch ( ... ) {
  }
}

__FIT_DECLSPEC
void NetTransport::connect( sockstream& s )
{
  try {
    Event ev;

    if ( pop( ev ) ) {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(manager()->_lock_tr);
        if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracenet) ) {
          *manager()->_trs << "Pid/ppid: " << std::tr2::getpid() << "/" << std::tr2::getppid() << "\n";
          manager()->dump( *manager()->_trs ) << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE

#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(manager()->_lock_tr);
        if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
          *manager()->_trs << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE
      manager()->push( ev );
    }
  }
  catch ( ios_base::failure& ex ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " " << ex.what()
                         << " (" << s.rdbuf()->inet_addr() << ":" << s.rdbuf()->port()
                         << ")" << endl;
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " unknown exception"
                         << " (" << s.rdbuf()->inet_addr() << ":" << s.rdbuf()->port()
                         << ")" << endl;
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
  return badaddr;
}

void NetTransportMgr::_loop( NetTransportMgr* p )
{
  NetTransportMgr& me = *p;
  Event ev;

  try {
    while ( me.pop( ev ) ) {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(manager()->_lock_tr);
        if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracenet) ) {
          *manager()->_trs << "Pid/ppid: " << std::tr2::getpid() << "/" << std::tr2::getppid() << "\n";
          manager()->dump( *manager()->_trs ) << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE

#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(manager()->_lock_tr);
        if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
          ios_base::fmtflags f = manager()->_trs->flags( ios_base::hex | ios_base::showbase );
          *manager()->_trs << "NetTransportMgr " << me.self_id() << " accept event "
                           << ev.code() << endl;
          manager()->_trs->flags( f );
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE
      manager()->push( ev );
    }
    me.NetTransport_base::_close();
#ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
        ios_base::fmtflags f = manager()->_trs->flags( ios_base::hex | ios_base::showbase );
        *manager()->_trs << "NetTransportMgr " << me.self_id() << " loop exit" << endl;
        manager()->_trs->flags( f );
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_STEM_TRACE
  }
  catch ( ... ) {
    me.NetTransport_base::_close();
#ifdef __FIT_STEM_TRACE
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
        ios_base::fmtflags f = manager()->_trs->flags( ios_base::hex | ios_base::showbase );
        *manager()->_trs << "NetTransportMgr " << me.self_id() << " loop exit due to exception" << endl;
        manager()->_trs->flags( f );
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

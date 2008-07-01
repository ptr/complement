// -*- C++ -*- Time-stamp: <08/07/01 16:05:40 yeti>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2008
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

#define MT_IO_REENTRANT( s )   // MT_REENTRANT( (s).rdbuf()->_M_lock, _0 );
#define MT_IO_REENTRANT_W( s ) // MT_REENTRANT( (s).rdbuf()->_M_lock_w, _0 );
#define MT_IO_LOCK_W( s )      // (s).rdbuf()->_M_lock_w.lock();
#define MT_IO_UNLOCK_W( s )    // (s).rdbuf()->_M_lock_w.unlock();


__FIT_DECLSPEC
void dump( std::ostream& o, const EDS::Event& e )
{
  MT_IO_REENTRANT( o )
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
  if ( net != 0 ) {
    manager()->Remove( this );
    net->close();
  }
}

bool NetTransport_base::pop( Event& _rs, gaddr_type& dst, gaddr_type& src )
{
  const int bsz = 2+(4+2+1)*2+4;
  uint32_t buf[bsz];
  using namespace std;

  if ( !net->read( (char *)buf, sizeof(uint32_t) ).good() ) {
    return false;
  }

  if ( buf[0] != EDS_MAGIC ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " StEM Magic fail ("
                         << net->rdbuf()->inet_addr() << ":" << net->rdbuf()->port() << ")"
                         << endl;
      }
    }
    catch ( ... ) {
    }

    NetTransport_base::close();
    return false;
  }

  if ( !net->read( (char *)&buf[1], sizeof(uint32_t) * (bsz-1) ).good() ) {
    return false;
  }
  _rs.code( from_net( buf[1] ) );
  dst._xnet_unpack( (const char *)&buf[2] );
  src._xnet_unpack( (const char *)&buf[9] );
  uint32_t _x_count = from_net( buf[16] );
  _rs.resetf( from_net( buf[17] ) );
  uint32_t sz = from_net( buf[18] );

  if ( sz >= EDS_MSG_LIMIT ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " StEM Message size too big: " << sz
                         << " (" << net->rdbuf()->inet_addr() << ":" << net->rdbuf()->port()
                         << ")" << endl;
      }
    }
    catch ( ... ) {
    }
    NetTransport_base::close();
    return false;
  }

  adler32_type adler = adler32( (unsigned char *)buf, sizeof(uint32_t) * 19 );
  if ( adler != from_net( buf[19] ) ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " StEM Adler-32 fail"
                         << " (" << net->rdbuf()->inet_addr() << ":" << net->rdbuf()->port()
                         << ")" << endl;
      }
    }
    catch ( ... ) {
    }

    NetTransport_base::close();
    return false;
  }

  string& str = _rs.value();

  str.erase();
  str.reserve( sz );
  while ( sz-- > 0 ) {
    str += (char)net->get();
  }
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

  return net->good();
}


__FIT_DECLSPEC
bool NetTransport_base::push( const Event& _rs, const gaddr_type& dst, const gaddr_type& src )
{
#ifdef __FIT_STEM_TRACE
  try {
    lock_guard<mutex> lk(manager()->_lock_tr);
    if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracenet)) ) {
      int flags = manager()->_trs->flags();
      *manager()->_trs << "\tMessage to remote " << hex << showbase << _rs.code() << " "
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

  if ( !net->good() ) {
    return false;
  }
  const int bsz = 2+(4+2+1)*2+4;
  uint32_t buf[bsz];

  buf[0] = EDS_MAGIC;
  buf[1] = to_net( _rs.code() );
  dst._xnet_pack( reinterpret_cast<char *>(buf + 2) );
  src._xnet_pack( reinterpret_cast<char *>(buf + 9) );

  MT_IO_LOCK_W( *net )

  buf[16] = to_net( ++_count );
  buf[17] = to_net( _rs.flags() );
  buf[18] = to_net( static_cast<uint32_t>(_rs.value().size()) );
  buf[19] = to_net( adler32( (unsigned char *)buf, sizeof(uint32_t) * 19 )); // crc

  try {
    net->write( (const char *)buf, sizeof(uint32_t) * 20 );
        
    copy( _rs.value().begin(), _rs.value().end(),
          ostream_iterator<char,char,char_traits<char> >(*net) );

    net->flush();
    if ( !net->good() ) {
      throw ios_base::failure( "net not good" );
    }
  }
  catch ( ios_base::failure& ) {
    if ( net != 0 ) { // clear connection: required by non-Solaris OS
      net->close();
    }
  }
  catch ( ... ) {
    MT_IO_UNLOCK_W( *net )
    throw;
  }
  MT_IO_UNLOCK_W( *net )

  return net->good();
}

__FIT_DECLSPEC
NetTransport::NetTransport( std::sockstream& s ) :
    NetTransport_base( "stem::NetTransport" ),
    _handshake( false )
{
  net = &s;
}

__FIT_DECLSPEC
void NetTransport::_do_handshake()
{
  try {
    Event ev;
    gaddr_type dst;
    gaddr_type src;

    if ( pop( ev, dst, src ) ) {
      if ( ev.code() == EV_STEM_TRANSPORT ) {
        src.addr = ns_addr;
        addr_type xsrc = manager()->reflect( src );
        if ( xsrc == badaddr || (xsrc & extbit) ) { // ignore, if local; but add new transport otherwise
          manager()->SubscribeRemote( detail::transport( static_cast<NetTransport_base *>(this), detail::transport::socket_tcp, 10 ), src );
        }
        src.addr = default_addr;
        xsrc = manager()->reflect( src );
        if ( xsrc == badaddr || (xsrc & extbit) ) { // ignore, if local; but add new transport otherwise
          manager()->SubscribeRemote( detail::transport( static_cast<NetTransport_base *>(this), detail::transport::socket_tcp, 10 ), src );
        }
        Event ack( EV_STEM_TRANSPORT_ACK );
        if ( !push( ack, src, self_glid() ) ) { // the source (second arg) is NetTransport
          throw runtime_error( "net handshake error" );
        }
      } else {
        throw runtime_error( "net handshake error" );
      }
    } else {
      throw runtime_error( "net error or net handshake error" );
    }
    _handshake = true;
  }
  catch ( runtime_error& err ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " " << err.what()
                         << " (" << net->rdbuf()->inet_addr() << ":" << net->rdbuf()->port()
                         << ")" << endl;
      }
    }
    catch ( ... ) {
    }

    net->close();
  }
  catch ( ... ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " unknown exception"
                         << " (" << net->rdbuf()->inet_addr() << ":" << net->rdbuf()->port()
                         << ")" << endl;
      }
    }
    catch ( ... ) {
    }

    net->close();
  }
}

__FIT_DECLSPEC
void NetTransport::connect( sockstream& s )
{
  if ( !_handshake ) {
    _do_handshake();
    return;
  }

  try {
    Event ev;
    gaddr_type dst;
    gaddr_type src;

    if ( pop( ev, dst, src ) ) {
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
      addr_type xdst = manager()->reflect( dst );
      if ( xdst == badaddr ) {
#ifdef __FIT_STEM_TRACE
        try {
          lock_guard<mutex> lk(manager()->_lock_tr);
          if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracefault)) ) {
            *manager()->_trs << __FILE__ << ":" << __LINE__
                             << " ("
                             << std::tr2::getpid() << "/" << std::tr2::getppid() << ") "
                             << "Unknown destination\n";
            manager()->dump( *manager()->_trs ) << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_STEM_TRACE
        return;
      }
      ev.dest( xdst );
      addr_type xsrc = manager()->reflect( src );
      if ( xsrc == badaddr ) {
        ev.src( manager()->SubscribeRemote( detail::transport( static_cast<NetTransport_base *>(this), detail::transport::socket_tcp, 10 ), src ) );
      } else {
        ev.src( xsrc );
      }
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

__FIT_DECLSPEC
addr_type NetTransportMgr::open( const char *hostname, int port,
                                 std::sock_base::stype stype )
{
  try {
    // I should be sure, that not more then one _loop running from here!
    // For this, I enforce close connection before I try open new,
    // and wait thread with _loop before start new.
    if ( _channel.is_open() ) {
      close(); // I should wait termination of _loop, clear EDS address mapping, etc.
    }
    join(); // This is safe: transparent if no _loop, and wait it if one exist
    _channel.open( hostname, port, stype );

    if ( _channel.good() ) {
      Event ev( EV_STEM_TRANSPORT );
      gaddr_type dst;
      gaddr_type src;
      addr_type xsrc = badaddr;
      if ( !push( ev, gaddr_type(), self_glid() ) ) {
        throw runtime_error( "net error or net handshake error" );
      }
      if ( pop( ev, dst, src ) ) {
        if ( ev.code() == EV_STEM_TRANSPORT_ACK ) {
          src.addr = ns_addr;
          xsrc = manager()->reflect( src );
          // indeed src is something like NetTransport, so substitute ns:
          src.addr = ns_addr;
          if ( xsrc == badaddr || (xsrc & extbit)) { // ignore local; but add new transport otherwise
            manager()->SubscribeRemote( detail::transport( static_cast<NetTransport_base *>(this), detail::transport::socket_tcp, 10 ), src );
          }
          // indeed src is something like NetTransport, so substitute default:
          src.addr = default_addr;
          xsrc = manager()->reflect( src );
          if ( xsrc == badaddr || (xsrc & extbit)) { // ignore local; but add new transport otherwise
            xsrc = manager()->SubscribeRemote( detail::transport( static_cast<NetTransport_base *>(this), detail::transport::socket_tcp, 10 ), src );
          }
        } else {
          throw runtime_error( "net handshake error" );
        }
      } else {
        throw runtime_error( "net error or net handshake error" );
      }
      _thr = new std::tr2::thread( _loop, this );
      return xsrc; // zero_object;
    }
  }
  catch ( runtime_error& err ) {
    try {
      lock_guard<mutex> lk(manager()->_lock_tr);
      if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & EvManager::tracefault) ) {
        *manager()->_trs << __FILE__ << ":" << __LINE__ << " " << err.what()
                         << " (" << _channel.rdbuf()->inet_addr() << ":" << _channel.rdbuf()->port()
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
                         << " (" << _channel.rdbuf()->inet_addr() << ":" << _channel.rdbuf()->port()
                         << ")" << endl;
      }
    }
    catch ( ... ) {
    }
  }
  return badaddr;
}

__FIT_DECLSPEC
void NetTransportMgr::_close()
{
  _channel.rdbuf()->shutdown( sock_base::stop_in | sock_base::stop_out );
  // _channel.rdbuf()->shutdown( sock_base::stop_in );
  NetTransport_base::_close();
  // _channel.close();
  join();
}

void NetTransportMgr::_loop( NetTransportMgr* p )
{
  NetTransportMgr& me = *p;
  Event ev;
  gaddr_type dst;
  gaddr_type src;

  try {
    while ( p->pop( ev, dst, src ) ) {
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
      addr_type xdst = manager()->reflect( dst );
      if ( xdst == badaddr ) {
#ifdef __FIT_STEM_TRACE
        try {
          lock_guard<mutex> lk(manager()->_lock_tr);
          if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracefault)) ) {
            *manager()->_trs << __FILE__ << ":" << __LINE__
                             << " ("
                             << std::tr2::getpid() << "/" << std::tr2::getppid() << ") "
                             << "Unknown destination\n";
            manager()->dump( *manager()->_trs ) << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_STEM_TRACE
        continue;
      }
      ev.dest( xdst );
      addr_type xsrc = manager()->reflect( src );
      if ( xsrc == badaddr ) {
        ev.src( manager()->SubscribeRemote( detail::transport( static_cast<NetTransport_base *>(&me), detail::transport::socket_tcp, 10 ), src ) );
      } else {
        ev.src( xsrc );
      }
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
    p->NetTransport_base::close();
  }
  catch ( ... ) {
    p->NetTransport_base::close();
    // throw;
  }
}

#if 0
__FIT_DECLSPEC
void NetTransportMP::connect( sockstream& s )
{
  Event ev;
  gaddr_type dst;
  gaddr_type src;

  try {
    if ( pop( ev, dst, src ) ) {
#ifdef __FIT_STEM_TRACE
      try {
        lock_guard<mutex> lk(manager()->_lock_tr);
        if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & E
vManager::tracenet) ) {
          *manager()->_trs << "Pid/ppid: " << std::tr2::getpid() << "/" << std::tr2::getppid() <<
"\n";
          manager()->dump( *manager()->_trs ) << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_STEM_TRACE
      addr_type xdst = manager()->reflect( dst );
      if ( xdst == badaddr ) {
#ifdef __FIT_STEM_TRACE
        try {
          lock_guard<mutex> lk(manager()->_lock_tr);
          if ( manager()->_trs != 0 && manager()->_trs->good() && (manager()->_trflags & (EvManager::tracefault)) ) {
            *manager()->_trs << __FILE__ << ":" << __LINE__
                             << " ("
                             << std::tr2::getpid() << "/" << std::tr2::getppid() << ") "
                             << "Unknown destination\n";
            manager()->dump( *manager()->_trs ) << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_STEM_TRACE
        return;
      }
      ev.dest( xdst );
      addr_type xsrc = manager()->reflect( src );
      if ( xsrc == badaddr ) {
        detail::transport tr( static_cast<NetTransport_base *>(this), detail::transport::socket_tcp, 10 );
        ev.src( manager()->SubscribeRemote( tr, src ) );
      } else {
        ev.src( xsrc );
      }
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
    if ( !s.good() ) {
      throw ios_base::failure( "sockstream not good" );
    }
  }
  catch ( ... ) {
    this->close(); // clear connection
    net = 0;
  }
}

#endif

} // namespace stem

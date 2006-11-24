// -*- C++ -*- Time-stamp: <06/11/24 13:09:57 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
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
#include <mt/xmt.h>

const uint32_t EDS_MSG_LIMIT = 0x400000; // 4MB

namespace stem {

using namespace std;

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

__FIT_DECLSPEC NetTransport_base::~NetTransport_base()
{
  NetTransport_base::close();
}

__FIT_DECLSPEC void NetTransport_base::close()
{
  // cerr << __FILE__ << ":" << __LINE__ << endl;
  if ( net != 0 ) {
    manager()->Remove( this );
    net->close();
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    net = 0;
  }
}

bool NetTransport_base::pop( Event& _rs )
{
  uint32_t buf[8];
  using namespace std;

  // _STLP_ASSERT( net != 0 );

  // cerr << __FILE__ << ":" << __LINE__ << endl;
  MT_IO_REENTRANT( *net )

  if ( !net->read( (char *)buf, sizeof(uint32_t) ).good() ) {
    return false;
  }
  // cerr << __FILE__ << ":" << __LINE__ << endl;

  // if ( from_net( buf[0] ) != EDS_MAGIC ) {
  if ( buf[0] != EDS_MAGIC ) {
    cerr << "EDS Magic fail" << endl;
    NetTransport_base::close();
    return false;
  }

  if ( !net->read( (char *)&buf[1], sizeof(uint32_t) * 7 ).good() ) {
    return false;
  }
  _rs.code( from_net( buf[1] ) );
  _rs.dest( from_net( buf[2] ) );
  _rs.src( from_net( buf[3] ) );
  uint32_t _x_count = from_net( buf[4] );
  uint32_t _x_time = from_net( buf[5] ); // time?
  uint32_t sz = from_net( buf[6] );

  if ( sz >= EDS_MSG_LIMIT ) {
    cerr << "EDS Message size too big: " << sz << endl;
    NetTransport_base::close();
    return false;
  }

  adler32_type adler = adler32( (unsigned char *)buf, sizeof(uint32_t) * 7 );
  if ( adler != from_net( buf[7] ) ) {
    cerr << "EDS Adler-32 fail" << endl;
    NetTransport_base::close();
    return false;
  }

  string& str = _rs.value();

  str.erase();  // str.clear(); absent in VC's STL
  str.reserve( sz );
#if defined(_MSC_VER) && (_MSC_VER < 1200)
  char ch;
#endif
  while ( sz-- > 0 ) {
#if defined(_MSC_VER) && (_MSC_VER < 1200)
    net->get( ch );
    str += ch;
#else
    str += (char)net->get();
#endif
  }

  return net->good();
}


__FIT_DECLSPEC
bool NetTransport_base::push( const Event& _rs, const gaddr_type& dst, const gaddr_type& src )
{
  // _STLP_ASSERT( net != 0 );
  if ( !net->good() ) {
    return false;
  }
  // const int bsz = 8-2+(4+2+1)*2;
  // uint32_t buf[bsz];

  ostringstream sbuf;                                        // 4 bytes
  sbuf.write( (const char *)&EDS_MAGIC, sizeof(EDS_MAGIC) ); // 0
  __pack_base::__net_pack( sbuf, _rs.code() );               // 1
  dst.net_pack( sbuf );                                      // 2-8
  src.net_pack( sbuf );                                      // 9-15

  // MT_IO_REENTRANT_W( *net )
  MT_IO_LOCK_W( *net )

  __pack_base::__net_pack( sbuf, ++_count );                 // 16
  __pack_base::__net_pack( sbuf, 0 );                        // 17 time?
  __pack_base::__net_pack( sbuf, static_cast<uint32_t>(_rs.value().size()) ); // 18
  __pack_base::__net_pack( sbuf, adler32( (unsigned char *)sbuf.str().c_str(), sizeof(uint32_t) * 19 ) ); // 19 crc

  try {
    net->write( sbuf.str().c_str(), sizeof(uint32_t) * 20 );
        
    copy( _rs.value().begin(), _rs.value().end(),
          ostream_iterator<char,char,char_traits<char> >(*net) );

    net->flush();
    if ( !net->good() ) {
      throw ios_base::failure( "net not good" );
    }
  }
  catch ( ios_base::failure& ) {
    if ( net != 0 ) { // clear connection: required by non-Solaris OS
      rar.clear();    // for MP connection policy  
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
    NetTransport_base( "stem::NetTransport" )
{
}

__FIT_DECLSPEC
void NetTransport::connect( sockstream& s )
{
  Event ev;

  try {
    if ( pop( ev ) ) {
      ev.src( rar_map( ev.src(), _at_hostname ) ); // substitute my local id
      manager()->push( ev );
    }
  }
  catch ( ios_base::failure& ex ) {
    cerr << ex.what() << endl;
  }
  catch ( ... ) {
    s.close();
  }
  // cerr << "Disconnected" << endl;
}

// connect initiator (client) function

__FIT_DECLSPEC
NetTransportMgr::~NetTransportMgr()
{
  // cerr << __FILE__ << ":" << __LINE__ << endl;
  if ( net ) {
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    net->rdbuf()->shutdown( sock_base::stop_in | sock_base::stop_out );
    net->close(); // otherwise _loop may not exited
    // this->close();
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    join();
    // NetTransport_base::close() called during loop thread termination (see _loop)
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    delete net;
    net = 0;
  }        
}

__FIT_DECLSPEC
addr_type NetTransportMgr::open( const char *hostname, int port,
                                 std::sock_base::stype stype )
{
  // I should be sure, that not more then one _loop running from here!
  // For this, I enforce close connection before I try open new,
  // and wait thread with _loop before start new.
  if ( net == 0 ) {
    net = new sockstream( hostname, port, stype );
  } else if ( net->is_open() ) {
    // net->close();
    close(); // I should wait termination of _loop, clear EDS address mapping, etc.
    net->open( hostname, port, stype );
  } else {
    join(); // This is safe: transparent if no _loop, and wait it if one exist
    net->open( hostname, port, stype );
  }

  if ( net->good() ) {
    _net_ns = rar_map( ns_addr, __ns_at + hostname );
    addr_type zero_object = rar_map( 0, __at + hostname );
    _thr.launch( _loop, this, 0, PTHREAD_STACK_MIN * 2 ); // start thread here
    return zero_object;
  }
  return badaddr;
}

__FIT_DECLSPEC
void NetTransportMgr::close()
{
  if ( net ) {
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    net->rdbuf()->shutdown( sock_base::stop_in | sock_base::stop_out );
    net->close(); // otherwise _loop may not exited
    // this->close();
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    join();
    // NetTransport_base::close() called during loop thread termination (see _loop)
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    delete net;
    net = 0;
  }        
}

xmt::Thread::ret_code NetTransportMgr::_loop( void *p )
{
  NetTransportMgr& me = *reinterpret_cast<NetTransportMgr *>(p);
  heap_type::iterator r;
  Event ev;
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  try {
    while ( me.pop( ev ) ) {
      ev.src( me.rar_map( ev.src(), __at + hostname( me.net->rdbuf()->inet_addr()) ) ); // substitute my local id
      manager()->push( ev );
    }
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    me.NetTransport_base::close();
  }
  catch ( ... ) {
    me.NetTransport_base::close();
    // throw;
    rt.iword = -1;
  }

  return rt;
}

__FIT_DECLSPEC
void NetTransportMP::connect( sockstream& s )
{
  const string& _hostname = hostname( s.rdbuf()->inet_addr() );
  // bool sock_dgr = (s.rdbuf()->stype() == std::sock_base::sock_stream) ? false : true;

  Event ev;
  // cerr << "Connected: " << _hostname << endl;

  try {
    if ( pop( ev ) ) {
      ev.src( rar_map( ev.src(), __at + _hostname ) ); // substitute my local id
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
  // cerr << "Connected: " << _hostname << endl;
}

} // namespace stem

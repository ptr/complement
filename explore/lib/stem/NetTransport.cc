// -*- C++ -*- Time-stamp: <06/07/12 23:19:41 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include <iterator>
#include <iomanip>
#include "stem/NetTransport.h"
#include "stem/EventHandler.h"
#include "stem/EvManager.h"
#include "crc.h"
#include "stem/EDSEv.h"
#include <mt/xmt.h>

const unsigned EDS_MSG_LIMIT = 0x400000; // 4MB

namespace stem {

#ifndef _MSC_VER
using namespace std;
#endif

#ifdef _BIG_ENDIAN
static const unsigned EDS_MAGIC = 0xc2454453U;
#elif defined(_LITTLE_ENDIAN)
static const unsigned EDS_MAGIC = 0x534445c2U;
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

__FIT_DECLSPEC EvSessionManager NetTransport_base::smgr;

void NetTransport_base::disconnect()
{
  if ( _sid == badkey ) {
    _count = 0;
    return;
  }
  try {
    smgr.lock();
    if ( smgr.unsafe_is_avail( _sid ) ) {
      SessionInfo& info = smgr[_sid];
      info.disconnect();
//    cerr << "EvManager::disconnect: " << _sid << endl;
      if ( info._control != badaddr ) {
        Event_base<key_type> ev_disconnect( EV_EDS_DISCONNECT, _sid );
        ev_disconnect.dest( info._control );
        _sid = badkey;
        _count = 0;
        smgr.unlock();
//      cerr << "EvManager::disconnect, info._control: " << info._control << endl;
        Send( Event_convert<key_type>()(ev_disconnect) );
//      cerr << "===== Pass" << endl;
        return; // required: smgr.unlock() done.
      }
      smgr.unsafe_erase( _sid );
    }
    _sid = badkey;
    _count = 0;
    smgr.unlock();
  }
  catch ( ... ) {
    smgr.unlock();
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
    disconnect();
    rar.clear();
    net->close();
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    net = 0;
  }
}

__FIT_DECLSPEC
void NetTransport_base::establish_session( std::sockstream& s ) throw (std::domain_error)
{
  smgr.lock();
  _sid = smgr.unsafe_create();
  if ( _sid == badkey ) {
    smgr.unlock();
    throw std::domain_error( "bad session id" );
  }
  smgr[_sid]._host = hostname( s.rdbuf()->inet_addr() );
  smgr[_sid]._port = s.rdbuf()->port();
  smgr.unlock();
}

void NetTransport_base::mark_session_onoff( bool f )
{
  smgr.lock();
  if ( smgr.unsafe_is_avail( _sid ) ) {
    if ( f ) 
      smgr[ _sid ].connect();
    else
      smgr[ _sid ].disconnect();
  }
  smgr.unlock();
}

const string __ns_at( "ns@" );
const string __at( "@" );

addr_type NetTransport_base::rar_map( addr_type k, const string& name )
{
  heap_type::iterator r = rar.find( k );
  if ( r == rar.end() ) {
    r = rar.insert(
      heap_type::value_type( k,
                             manager()->SubscribeRemote( this, k, name ) ) ).first;
  }

  return (*r).second;
}

bool NetTransport_base::pop( Event& _rs )
{
  unsigned buf[8];
  using namespace std;

  // _STLP_ASSERT( net != 0 );

  // cerr << __FILE__ << ":" << __LINE__ << endl;
  MT_IO_REENTRANT( *net )

  if ( !net->read( (char *)buf, sizeof(unsigned) ).good() ) {
    // cerr << __FILE__ << ":" << __LINE__ << endl;
    return false;
  }
  // cerr << __FILE__ << ":" << __LINE__ << endl;

  // if ( from_net( buf[0] ) != EDS_MAGIC ) {
  if ( buf[0] != EDS_MAGIC ) {
    cerr << "EDS Magic fail" << endl;
    NetTransport_base::close();
    return false;
  }

  if ( !net->read( (char *)&buf[1], sizeof(unsigned) * 7 ).good() ) {
    return false;
  }
  _rs.code( from_net( buf[1] ) );
  _rs.dest( from_net( buf[2] ) );
  _rs.src( from_net( buf[3] ) );
  unsigned _x_count = from_net( buf[4] );
  unsigned _x_time = from_net( buf[5] ); // time?
  unsigned sz = from_net( buf[6] );

  if ( sz >= EDS_MSG_LIMIT ) {
    cerr << "EDS Message size too big: " << sz << endl;
    NetTransport_base::close();
    return false;
  }

  adler32_type adler = adler32( (unsigned char *)buf, sizeof(unsigned) * 7 );
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

  if ( _sid != badkey ) {
    smgr.lock();
    if ( smgr.unsafe_is_avail( _sid ) ) {
      SessionInfo& sess = smgr[_sid];
      sess.inc_from( 8 * sizeof(unsigned) + str.size() );
      if ( sess._un_from != _x_count ) {
        cerr << "EDS Incoming event(s) lost, or missrange event: " << sess._un_from
             << ", " << _x_count << " (Session: " << _sid << ") --- ";
        cerr << endl;
        sess._un_from = _x_count; // Retransmit?    
      }
    }
    smgr.unlock();
  }

  return net->good();
}


__FIT_DECLSPEC
bool NetTransport_base::push( const Event& _rs )
{
  // _STLP_ASSERT( net != 0 );
  if ( _sid == badkey || !net->good() ) {
    return false;
  }
  unsigned buf[8];

  // buf[0] = to_net( EDS_MAGIC );
  buf[0] = EDS_MAGIC;
  buf[1] = to_net( _rs.code() );
  buf[2] = to_net( _rs.dest() );
  buf[3] = to_net( _rs.src() );

  // MT_IO_REENTRANT_W( *net )
  MT_IO_LOCK_W( *net )

  buf[4] = to_net( ++_count );
  buf[5] = 0; // time?
  buf[6] = to_net( _rs.value().size() );
  buf[7] = to_net( adler32( (unsigned char *)buf, sizeof(unsigned) * 7 ) ); // crc

  try {
    net->write( (const char *)buf, sizeof(unsigned) * 8 );
        
    copy( _rs.value().begin(), _rs.value().end(),
          ostream_iterator<char,char,char_traits<char> >(*net) );

    net->flush();
    if ( _sid != badkey && net->good() ) {
      smgr.lock();
      if ( smgr.unsafe_is_avail( _sid ) ) {
        SessionInfo& sess = smgr[_sid];
        sess.inc_to( 8 * sizeof(unsigned) + _rs.value().size() );
        if ( sess._un_to != _count ) {
          cerr << "Outgoing event(s) lost, or missrange event: " << sess._un_to
               << ", " << _count << " (Session " << _sid << ")" << endl;
          // kill( getpid(), SIGQUIT );
        }
      }
      smgr.unlock();
    } else {
      throw ios_base::failure( "net not good" );
    }
  }
  catch ( ios_base::failure& ) {
    if ( net != 0 ) { // clear connection: required by non-Solaris OS
      disconnect();   // for MP connection policy  
      rar.clear();
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
  const string& _hostname = hostname( s.rdbuf()->inet_addr() );
  // cerr << "Connected: " << _hostname << endl;
  s.setoptions( std::sock_base::so_linger, true, 10 );
  s.setoptions( std::sock_base::so_keepalive, true );

  net = &s;
  _at_hostname = __at + _hostname;

  try {
    establish_session( s );
    _net_ns = rar_map( ns_addr, __ns_at + _hostname );
  }
  catch ( std::domain_error& ex ) {
    cerr << ex.what() << endl;
  }
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
    // disconnect();
    // throw;
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
  net->setoptions( std::sock_base::so_linger, true, 10 );
  net->setoptions( std::sock_base::so_keepalive, true );

  if ( net->good() ) {
    _net_ns = rar_map( ns_addr, __ns_at + hostname );
    addr_type zero_object = rar_map( 0, __at + hostname );
    _sid = smgr.create();
    _thr.launch( _loop, this ); // start thread here
    return zero_object;
  }
  return badaddr;
}

__FIT_DECLSPEC
void NetTransportMgr::close()
{
  // cerr << __FILE__ << ":" << __LINE__ << endl;
  NetTransport_base::close();
  join(); // I should wait termination of _loop
}

int NetTransportMgr::_loop( void *p )
{
  NetTransportMgr& me = *reinterpret_cast<NetTransportMgr *>(p);
  heap_type::iterator r;
  Event ev;

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
  }

  return 0;  
}

__FIT_DECLSPEC addr_type NetTransportMgr::make_map( addr_type k, const char *name )
{
  string full_name = name;
  full_name += __at;
  full_name += hostname( net->rdbuf()->inet_addr() );

  return rar_map( k, full_name );
}

__FIT_DECLSPEC
void NetTransportMP::connect( sockstream& s )
{
  const string& _hostname = hostname( s.rdbuf()->inet_addr() );
  bool sock_dgr = (s.rdbuf()->stype() == std::sock_base::sock_stream) ? false : true;

  Event ev;
  // cerr << "Connected: " << _hostname << endl;

  try {
    if ( _sid == badkey ) {
      establish_session( s );
      net = &s;
// #ifndef __hpux
      if ( !sock_dgr ) {
        net->setoptions( std::sock_base::so_linger, true, 10 );
        net->setoptions( std::sock_base::so_keepalive, true );
      }
// #endif
    } else if ( sock_dgr /* && _sid != badkey */ ) {
      mark_session_onoff( true );
    }
    // indeed here need more check: data of event
    // and another: message can be break, and other datagram can be
    // in the middle of message...
    if ( pop( ev ) ) {
      ev.src( rar_map( ev.src(), __at + _hostname ) ); // substitute my local id
      manager()->push( ev );
    }
    if ( !s.good() ) {
      throw ios_base::failure( "sockstream not good" );
    }
    if ( sock_dgr && _sid != badkey ) {
      mark_session_onoff( false );
    }
  }
  catch ( ... ) {
    this->close(); // clear connection
    net = 0;
  }
  // cerr << "Connected: " << _hostname << endl;
}

} // namespace stem

// -*- C++ -*- Time-stamp: <99/10/06 10:16:03 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ident "$SunId$ %Q%"

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#ifdef WIN32
#  ifdef _DLL
#    define __EDS_DLL __declspec( dllexport )
#  else
#    define __EDS_DLL
#  endif
#else
#  define __EDS_DLL
#endif

#include <config/feature.h>
#include <iomanip>
#include "EDS/NetTransport.h"
#include "EDS/EventHandler.h"
#include "EDS/EvManager.h"
#include "crc.h"
#include "EDS/EDSEv.h"

namespace EDS {

#ifndef _MSC_VER
using namespace std;
#endif


#define EDS_MAGIC 0xc2454453U

__EDS_DLL
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

EvSessionManager NetTransport_base::smgr;

void NetTransport_base::disconnect()
{
  if ( _sid != -1 && smgr.is_avail( _sid ) ) {
    SessionInfo& info = smgr[_sid];
    info.disconnect();
//    cerr << "EvManager::disconnect: " << _sid << endl;
    if ( info._control != Event::badaddr ) {
      Event_base<Event::key_type> ev_disconnect( EV_EDS_DISCONNECT, _sid );
      ev_disconnect.dest( info._control );
//      cerr << "EvManager::disconnect, info._control: " << info._control << endl;
      Send( EDS::Event_convert<Event::key_type>()(ev_disconnect) );
//      cerr << "===== Pass" << endl;
    } else {
      smgr.erase( _sid );
    }
    _sid = badkey;
    _count = 0;
  }
}

__EDS_DLL NetTransport_base::~NetTransport_base()
{
  manager()->Remove( this );
  disconnect();
}

__EDS_DLL void NetTransport_base::close()
{
  if ( net != 0 ) {
    manager()->Remove( this );    
    disconnect();
    rar.clear();
    net->close();
  }
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

bool NetTransport_base::pop( Event& _rs, SessionInfo& sess )
{
  unsigned buf[8];

  __stl_assert( net != 0 );
  if ( !net->read( (char *)buf, sizeof(unsigned) ).good() ) {
    return false;
  }

  if ( from_net( buf[0] ) != EDS_MAGIC ) {
    cerr << "Magic fail" << endl;
    close();
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

  adler32_type adler = adler32( (unsigned char *)buf, sizeof(unsigned) * 7 );
  if ( adler != from_net( buf[7] ) ) {
    cerr << "Adler-32 fail" << endl;
    close();
    return false;
  }

  string& str = _rs.value();

  str.erase();  // str.clear(); absent in VC's STL
  str.reserve( sz );
  while ( sz-- > 0 ) {
    str += (char)net->get();
  }

  sess.inc_from( 8 * sizeof(unsigned) + str.size() );
  if ( sess._un_from != _x_count ) {
    cerr << "Event(s) lost, or missrange event: " << sess._un_from
         << ", " << _x_count << "(" << _sid << ") --- ";
    cerr << endl;
    sess._un_from = _x_count; // Retransmit?    
  }

  return net->good();
}


__EDS_DLL
bool NetTransport_base::push( const Event& _rs )
{
  __stl_assert( net != 0 );
  unsigned buf[8];

  buf[0] = to_net( EDS_MAGIC );
  buf[1] = to_net( _rs.code() );
  buf[2] = to_net( _rs.dest() );
  buf[3] = to_net( _rs.src() );

  MT_REENTRANT( _lock, _1 );

  buf[4] = to_net( ++_count );

  SessionInfo& sess = smgr[_sid];
  sess.inc_to( 8 * sizeof(unsigned) + _rs.value().size() );

  buf[5] = 0; // time?
  buf[6] = to_net( _rs.value().size() );
  buf[7] = to_net( adler32( (unsigned char *)buf, sizeof(unsigned) * 7 ) ); // crc

  net->write( (const char *)buf, sizeof(unsigned) * 8 );

  copy( _rs.value().begin(), _rs.value().end(),
        ostream_iterator<char,char,char_traits<char> >(*net) );

  net->flush();

  if ( sess._un_to != _count ) {
    cerr << "Event(s) lost, or missrange event: " << sess._un_to
         << ", " << _count << "(" << _sid << ")" << endl;
  }

  return net->good();
}

__EDS_DLL
void NetTransport::connect( sockstream& s )
{
  const string& hostname = s.rdbuf()->hostname();
  cerr << "Connected: " << hostname << endl;

  Event ev;
  _sid = smgr.create();
  SessionInfo& sess = smgr[_sid];

  sess._host = hostname;
  sess._port = s.rdbuf()->port();
  net = &s;
  const string _at_hostname( __at + hostname );

  try {
    _net_ns = rar_map( nsaddr, __ns_at + hostname );
    while ( pop( ev, sess ) ) {
      ev.src( rar_map( ev.src(), _at_hostname ) ); // substitute my local id
      manager()->Send( ev );
    }
    if ( !s.good() ) {
      s.close();
    }
  }
  catch ( ios_base::failure& ) {
    s.close();
    // Policy for NetTransport is thread per connect, so it's destructor
    // will be called bit later, and it do this.
    // disconnect();
  }
  catch ( ... ) {
    s.close();
    // disconnect();
    throw;
  }
  cerr << "Disconnected: " << hostname << endl;
}

// connect initiator (client) function

__EDS_DLL
addr_type NetTransportMgr::open( const char *hostname, int port,
                                 std::sock_base::stype stype )
{
  if ( net == 0 ) {
    net = new sockstream( hostname, port, stype );
  } else if ( net->is_open() ) {
    net->close();
    net->open( hostname, port, stype );
  } else {
    net->open( hostname, port, stype );
  }

  if ( net->good() ) {
    _net_ns = rar_map( nsaddr, __ns_at + hostname );
    addr_type zero_object = rar_map( 0, __at + hostname );
    _sid = smgr.create();
    _thr.launch( _loop, this ); // start thread here
    return zero_object;
  }
  return badaddr;
}

int NetTransportMgr::_loop( void *p )
{
  NetTransportMgr& me = *reinterpret_cast<NetTransportMgr *>(p);
  heap_type::iterator r;
  Event ev;

  SessionInfo& sess = smgr[me._sid];

  try {
    while ( me.pop( ev, sess ) ) {
      ev.src( me.rar_map( ev.src(), __at + me.net->rdbuf()->hostname() ) ); // substitute my local id
      manager()->Send( ev );
    }
    me.close();
  }
  catch ( ... ) {
    me.close();
    throw;
  }

  return 0;  
}

__EDS_DLL addr_type NetTransportMgr::make_map( addr_type k, const char *name )
{
  string full_name = name;
  full_name += __at;
  full_name += net->rdbuf()->hostname();

  return rar_map( k, full_name );
}

__EDS_DLL
void NetTransportMP::connect( sockstream& s )
{
  const string& hostname = s.rdbuf()->hostname();

  Event ev;
  bool first = false;
  if ( _sid == -1 ) {
    first = true;
    _sid = smgr.create();
  }

  SessionInfo& sess = smgr[ _sid ];

  try {
    if ( first ) {
      sess._host = hostname;
      sess._port = s.rdbuf()->port();
      net = &s;
    } else {
      sess.connect();
    }
    // indeed here need more check: data of event
    // and another: message can be break, and other datagram can be
    // in the middle of message...
    if ( pop( ev, sess ) ) {
      ev.src( rar_map( ev.src(), __at + hostname ) ); // substitute my local id
      manager()->Send( ev );
    }
    if ( !s.good() ) {
      throw ios_base::failure( "sockstream not good" );
    }
    sess.disconnect();
  }
  catch ( ... ) {
    s.close();
    disconnect();
  }
}

} // namespace EDS

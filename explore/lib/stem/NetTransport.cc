// -*- C++ -*- Time-stamp: <00/12/26 20:49:28 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics Ltd.
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#include <config/feature.h>
#include <iterator>
#include <iomanip>
#include "EDS/NetTransport.h"
#include "EDS/EventHandler.h"
#include "EDS/EvManager.h"
#include "crc.h"
#include "EDS/EDSEv.h"

#define EDS_MSG_LIMIT   0x100000 // 1MB

namespace EDS {

#ifndef _MSC_VER
using namespace std;
#endif

#ifdef _BIG_ENDIAN
// #  define EDS_MAGIC 0xc2454453U
static const unsigned EDS_MAGIC = 0xc2454453U;
#elif defined(_LITTLE_ENDIAN)
// #  define EDS_MAGIC 0x534445c2U
static const unsigned EDS_MAGIC = 0x534445c2U;
#else
#  error "Can't determine platform byte order!"
#endif

#ifdef __SGI_STL_OWN_IOSTREAMS

struct _auto_lock
{
    _STL_mutex& _M_lock;
  
    _auto_lock(_STL_mutex& __lock) : _M_lock(__lock)
      { _M_lock._M_acquire_lock(); }
    ~_auto_lock() { _M_lock._M_release_lock(); }

  private:
    void operator=(const _auto_lock&);
    _auto_lock(const _auto_lock&);
};

#  define MT_IO_REENTRANT( s )   _auto_lock __AutoLock( (s).rdbuf()->_M_lock );
#  define MT_IO_LOCK( s )        (s).rdbuf()->_M_lock._M_acquire_lock();
#  define MT_IO_UNLOCK( s )      (s).rdbuf()->_M_lock._M_release_lock();
#  define MT_IO_REENTRANT_W( s ) _auto_lock __AutoLock( (s).rdbuf()->_M_lock_w );
#  define MT_IO_LOCK_W( s )      (s).rdbuf()->_M_lock_w._M_acquire_lock();
#  define MT_IO_UNLOCK_W( s )    (s).rdbuf()->_M_lock_w._M_release_lock();
#else // !__SGI_STL_OWN_IOSTREAMS
#  define MT_IO_REENTRANT( s )
#  define MT_IO_LOCK( s )
#  define MT_IO_UNLOCK( s )
#  define MT_IO_REENTRANT_W( s )
#  define MT_IO_LOCK_W( s )
#  define MT_IO_UNLOCK_W( s )
#endif


__PG_DECLSPEC
void dump( __STD::ostream& o, const EDS::Event& e )
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

__PG_DECLSPEC EvSessionManager NetTransport_base::smgr;

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
        smgr.unsafe_erase( _sid );
        _sid = badkey;
        _count = 0;
        smgr.unlock();
//      cerr << "EvManager::disconnect, info._control: " << info._control << endl;
        Send( Event_convert<key_type>()(ev_disconnect) );
//      cerr << "===== Pass" << endl;
        return; // required: smgr.unlock() done.
      }
    }
    _sid = badkey;
    _count = 0;
    smgr.unlock();
  }
  catch ( ... ) {
    smgr.unlock();
  }
}

__PG_DECLSPEC NetTransport_base::~NetTransport_base()
{
  NetTransport_base::close();
}

__PG_DECLSPEC void NetTransport_base::close()
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

bool NetTransport_base::pop( Event& _rs )
{
  unsigned buf[8];

  __STL_ASSERT( net != 0 );

  MT_IO_REENTRANT( *net )

  if ( !net->read( (char *)buf, sizeof(unsigned) ).good() ) {
    return false;
  }

  // if ( from_net( buf[0] ) != EDS_MAGIC ) {
  if ( buf[0] != EDS_MAGIC ) {
    cerr << "Magic fail" << endl;
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
    cerr << "Message size too big: " << sz << endl;
    NetTransport_base::close();
    return false;
  }

  adler32_type adler = adler32( (unsigned char *)buf, sizeof(unsigned) * 7 );
  if ( adler != from_net( buf[7] ) ) {
    cerr << "Adler-32 fail" << endl;
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
        cerr << "Incoming event(s) lost, or missrange event: " << sess._un_from
             << ", " << _x_count << " (Session: " << _sid << ") --- ";
        cerr << endl;
        sess._un_from = _x_count; // Retransmit?    
      }
    }
    smgr.unlock();
  }

  return net->good();
}


__PG_DECLSPEC
bool NetTransport_base::push( const Event& _rs )
{
  __STL_ASSERT( net != 0 );
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
        }
      }
      smgr.unlock();
    }
  }
  catch ( ... ) {
    MT_IO_UNLOCK_W( *net )
    throw;
  }
  MT_IO_UNLOCK_W( *net )

  return net->good();
}

__PG_DECLSPEC
void NetTransport::connect( sockstream& s )
{
  const string& hostname = s.rdbuf()->hostname();
  cerr << "Connected: " << hostname << endl;
  s.rdbuf()->setoptions( std::sock_base::so_linger, true, 10 );
  s.rdbuf()->setoptions( std::sock_base::so_keepalive, true );

  Event ev;
  net = &s;
  const string _at_hostname( __at + hostname );

  try {
    smgr.lock();
    _sid = smgr.unsafe_create();
    if ( _sid == badkey ) {
      smgr.unlock();
      throw std::domain_error( "NetTransport::connect: bad session id" );
    }
    smgr[_sid]._host = hostname;
    smgr[_sid]._port = s.rdbuf()->port();
    smgr.unlock();

    _net_ns = rar_map( nsaddr, __ns_at + hostname );
    while ( pop( ev ) ) {
      ev.src( rar_map( ev.src(), _at_hostname ) ); // substitute my local id
      manager()->push( ev );
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

__PG_DECLSPEC
NetTransportMgr::~NetTransportMgr()
{
  if ( net ) {
    net->close(); // otherwise _loop may not exited
    // this->close();
    join();
    delete net;
    net = 0;
  }        
}

__PG_DECLSPEC
addr_type NetTransportMgr::open( const char *hostname, int port,
                                 __STD::sock_base::stype stype )
{
  if ( net == 0 ) {
    net = new sockstream( hostname, port, stype );
  } else if ( net->is_open() ) {
    net->close();
    net->open( hostname, port, stype );
  } else {
    net->open( hostname, port, stype );
  }
  net->rdbuf()->setoptions( std::sock_base::so_linger, true, 10 );
  net->rdbuf()->setoptions( std::sock_base::so_keepalive, true );

  if ( net->good() ) {
    _net_ns = rar_map( nsaddr, __ns_at + hostname );
    addr_type zero_object = rar_map( 0, __at + hostname );
    _sid = smgr.create();
    _thr.launch( _loop, this ); // start thread here
    return zero_object;
  }
  return badaddr;
}

__PG_DECLSPEC
void NetTransportMgr::close()
{
  NetTransport_base::close();
  join();
}

int NetTransportMgr::_loop( void *p )
{
  NetTransportMgr& me = *reinterpret_cast<NetTransportMgr *>(p);
  heap_type::iterator r;
  Event ev;

  try {
    while ( me.pop( ev ) ) {
      ev.src( me.rar_map( ev.src(), __at + me.net->rdbuf()->hostname() ) ); // substitute my local id
      manager()->push( ev );
    }
    me.NetTransport_base::close();
  }
  catch ( ... ) {
    me.NetTransport_base::close();
    throw;
  }

  return 0;  
}

__PG_DECLSPEC addr_type NetTransportMgr::make_map( addr_type k, const char *name )
{
  string full_name = name;
  full_name += __at;
  full_name += net->rdbuf()->hostname();

  return rar_map( k, full_name );
}

__PG_DECLSPEC
void NetTransportMP::connect( sockstream& s )
{
  const string& hostname = s.rdbuf()->hostname();
  bool sock_dgr = (s.rdbuf()->stype() == std::sock_base::sock_stream) ?
                  false : true;

  Event ev;

  try {
    if ( _sid == badkey ) {
      smgr.lock();
      _sid = smgr.unsafe_create();
      smgr[ _sid ]._host = hostname;
      smgr[ _sid ]._port = s.rdbuf()->port();
      smgr.unlock();
      net = &s;
// #ifndef __hpux
      if ( !sock_dgr ) {
        net->rdbuf()->setoptions( std::sock_base::so_linger, true, 10 );
        net->rdbuf()->setoptions( std::sock_base::so_keepalive, true );
      }
// #endif
    } else if ( sock_dgr /* && _sid != badkey */ ) {
      smgr.lock();
      if ( smgr.unsafe_is_avail( _sid ) ) {
        smgr[ _sid ].connect();
      }
      smgr.unlock();
    }
    // indeed here need more check: data of event
    // and another: message can be break, and other datagram can be
    // in the middle of message...
    if ( pop( ev ) ) {
      ev.src( rar_map( ev.src(), __at + hostname ) ); // substitute my local id
      manager()->push( ev );
    }
    if ( !s.good() ) {
      throw ios_base::failure( "sockstream not good" );
    }
    if ( sock_dgr && _sid != badkey ) {
      smgr.lock();
      if ( smgr.unsafe_is_avail( _sid ) ) {
        smgr[ _sid ].disconnect();
      }
      smgr.unlock();
    }
  }
  catch ( ... ) {
    s.close();
    // disconnect();
    this->close();
  }
}

} // namespace EDS

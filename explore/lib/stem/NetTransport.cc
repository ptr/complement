// -*- C++ -*- Time-stamp: <99/05/28 18:17:34 ptr>

#ident "$SunId$ %Q%"

#include <iomanip>
#include <NetTransport.h>
#include <EventHandler.h>
#include <EvManager.h>
#include <crc.h>

namespace EDS {

#ifndef _MSC_VER
using namespace std;
#endif


#define EDS_MAGIC 0xc2454453U

__DLLEXPORT
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
      Event_base<Event::key_type> ev_disconnect( EV_DISCONNECT, _sid );
      ev_disconnect.dest( info._control );
//      cerr << "EvManager::disconnect, info._control: " << info._control << endl;
      EventHandler::manager()->Send( EDS::Event_convert<Event::key_type>()(ev_disconnect), Event::mgraddr );
//      cerr << "===== Pass" << endl;
    } else {
      smgr.erase( _sid );
    }
    _sid = -1;
    _count = 0;
  }
}

__DLLEXPORT NetTransport_base::~NetTransport_base()
{
  EventHandler::manager()->Remove( this );
  disconnect();
}

// passive side (server) function

void NetTransport_base::event_process( Event& ev, const string& hostname )
{
//      cerr << "------------------------>>>>>>>>\n";
//      dump( std::cerr, ev );
  __stl_assert( EventHandler::manager() != 0 );
  // if _mgr == 0, best choice is close connection...
  // sess.inc_from( sizeof(__Event_Base) + sizeof(std::string::size_type) +
  //               ev.value_size() );
  // if ( sess.un_from( ev.seq() ) != 0 ) {
  //   cerr << "Event(s) lost, or miss range event" << endl;
  // }
  heap_type::iterator r = rar.find( ev._src );
  if ( r == rar.end() ) {
    r = rar.insert(
      heap_type::value_type( ev._src,
                             EventHandler::manager()->SubscribeRemote( this, ev._src, hostname ) ) ).first;
//        cerr << "Create remote object: " << hex << (*r).second
//             << " [" << ev._src << "]\n";
  }
  ev._src = (*r).second; // substitute my local id

  // if ( ev.sid() != _sid ) {
  //   ev.sid( _sid );
  // }
        
  EventHandler::manager()->Dispatch( ev );
}

bool NetTransport_base::pop( Event& __rs, SessionInfo& sess )
{
  unsigned buf[8];

  __stl_assert( net != 0 );
  if ( !net->read( (char *)buf, sizeof(unsigned) ).good() ) {
    return false;
  }

  if ( from_net( buf[0] ) != EDS_MAGIC ) {
    cerr << "Magic fail" << endl;
    net->close();
    return false;
  }

  if ( !net->read( (char *)&buf[1], sizeof(unsigned) * 7 ).good() ) {
    return false;
  }
  __rs.code( from_net( buf[1] ) );
  __rs.dest( from_net( buf[2] ) );
  __rs.src( from_net( buf[3] ) );
  unsigned _x_count = from_net( buf[4] );
  unsigned _x_time = from_net( buf[5] ); // time?
  unsigned sz = from_net( buf[6] );

  adler32_type adler = adler32( (unsigned char *)buf, sizeof(unsigned) * 7 );
  if ( adler != from_net( buf[7] ) ) {
    cerr << "Adler-32 fail" << endl;
    net->close();
    return false;
  }

  string& str = __rs.value();

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

__DLLEXPORT
bool NetTransport_base::push( const Event& __rs, const Event::key_type& rmkey,
                              const Event::key_type& srckey )
{
  __stl_assert( net != 0 );
  unsigned buf[8];

  buf[0] = to_net( EDS_MAGIC );
  buf[1] = to_net( __rs.code() );
  buf[2] = to_net( rmkey );
  buf[3] = to_net( srckey );

  MT_REENTRANT( _lock, _1 );

  buf[4] = to_net( ++_count );

  SessionInfo& sess = smgr[_sid];
  sess.inc_to( 8 * sizeof(unsigned) + __rs.value().size() );

  buf[5] = 0; // time?
  buf[6] = to_net( __rs.value().size() );
  buf[7] = to_net( adler32( (unsigned char *)buf, sizeof(unsigned) * 7 ) ); // crc

  net->write( (const char *)buf, sizeof(unsigned) * 8 );

  copy( __rs.value().begin(), __rs.value().end(),
        ostream_iterator<char,char,char_traits<char> >(*net) );

  net->flush();

  if ( sess._un_to != _count ) {
    cerr << "Event(s) lost, or missrange event: " << sess._un_to
         << ", " << _count << "(" << _sid << ")" << endl;
  }

  return net->good();
}

__DLLEXPORT
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

  try {
    while ( pop( ev, sess ) ) {
      event_process( ev, hostname );
    }
    if ( !s.good() ) {
      s.close();
    }
  }
  catch ( ios_base::failure& e ) {
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

__DLLEXPORT
NetTransport_base::key_type NetTransportMgr::open(
                                             const string& hostname, int port,
                                             std::sock_base::stype stype )
{
  if ( net == 0 ) {
    net = new sockstream( hostname.c_str(), port, stype );
  } else if ( net->is_open() ) {
    net->close();
    net->open( hostname.c_str(), port, stype );
  }

  // _partner_name = hostname;
  if ( net->good() ) {
    heap_type::iterator r;

    // forward register remote object with ID 0
    r = rar.insert( 
      heap_type::value_type( 0,
                             EventHandler::manager()->SubscribeRemote( this, 0, net->rdbuf()->hostname() /* _partner_name */ ) ) ).first;
    _sid = smgr.create();
    _thr.launch( _loop, this ); // start thread here
    return (*r).second;
  }
  return -1;
}

int NetTransportMgr::_loop( void *p )
{
  NetTransportMgr& me = *reinterpret_cast<NetTransportMgr *>(p);
  heap_type::iterator r;
  Event ev;

  SessionInfo& sess = smgr[me._sid];

  try {
    while ( me.pop( ev, sess ) ) {
      // dump( std::cerr, ev );
      __stl_assert( EventHandler::manager() != 0 );
      // if _mgr == 0, best choice is close connection...
      r = me.rar.find( ev._src );
      if ( r == me.rar.end() ) {
        r = me.rar.insert( 
          heap_type::value_type( ev._src,
          EventHandler::manager()->SubscribeRemote( &me, ev._src, me.net->rdbuf()->hostname() ) ) ).first;
//        cerr << "Create remote object: " << hex << (*r).second
//             << " [" << ev._src << "]\n";
      }
      ev._src = (*r).second; // substitute my local id

      EventHandler::manager()->Dispatch( ev );
    }
    if ( me.net ) {
      me.net->close();
    }
    EventHandler::manager()->Remove( &me );
    me.disconnect();
  }
  catch ( ... ) {
    if ( me.net ) {
      me.net->close();
    }
    EventHandler::manager()->Remove( &me );
    me.disconnect();
    throw;
  }

  return 0;  
}

__DLLEXPORT
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
      event_process( ev, hostname );
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

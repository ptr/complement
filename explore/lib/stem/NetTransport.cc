// -*- C++ -*- Time-stamp: <99/04/16 15:33:00 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <iomanip>
#include <NetTransport.h>
#include <EventHandler.h>
#include <EvManager.h>

namespace EDS {

#ifndef _MSC_VER
using namespace std;
#endif

__DLLEXPORT
void dump( std::ostream& o, const EDS::Event& e )
{
  o << setiosflags(ios_base::showbase) << hex
    << "Code: " << e.code() << " Destination: " << e.dest() << " Source: " << e.src()
    << " SID: " << e.sid()
    << "\nSN: " << e.seq()
    << "\nRN: " << e.responce()
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
  if ( smgr.is_avail( _sid ) ) {
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
  }
}

__DLLEXPORT NetTransport_base::~NetTransport_base()
{
  EventHandler::manager()->Remove( this );
  disconnect();
}

// passive side (server) function

void NetTransport_base::event_process( Event& ev, SessionInfo& sess, const string& hostname )
{
//      cerr << "------------------------>>>>>>>>\n";
//      dump( std::cerr, ev );
  __stl_assert( EventHandler::manager() != 0 );
  // if _mgr == 0, best choice is close connection...
  sess.inc_from( sizeof(__Event_Base) + sizeof(std::string::size_type) +
                 ev.value_size() );
  if ( sess.un_from( ev.seq() ) != 0 ) {
    cerr << "Event(s) lost, or miss range event" << endl;
  }
  heap_type::iterator r = rar.find( ev._src );
  if ( r == rar.end() ) {
    r = rar.insert(
      heap_type::value_type( ev._src,
                             EventHandler::manager()->SubscribeRemote( this, ev._src, hostname ) ) ).first;
//        cerr << "Create remote object: " << hex << (*r).second
//             << " [" << ev._src << "]\n";
  }
  ev._src = (*r).second; // substitute my local id

  if ( ev.sid() != _sid ) {
    ev.sid( _sid );
  }
        
  EventHandler::manager()->Dispatch( ev );
}

bool NetTransport_base::pop( Event& __rs )
{
  unsigned buf[7];

  __stl_assert( net != 0 );
  if ( !net->read( (char *)&buf, sizeof(unsigned) * 7 ).good() ) {
    return false;
  }
  __rs.code( from_net( buf[0] ) );
  __rs.dest( from_net( buf[1] ) );
  __rs.src( from_net( buf[2] ) );
  __rs.sid( from_net( buf[3] ) );
  __rs.seq( from_net( buf[4] ) );
  __rs.responce( from_net( buf[5] ) );

  unsigned sz = from_net( buf[6] );
  string& str = __rs.value();

  str.erase();  // str.clear(); absent in VC's STL
  str.reserve( sz );
  while ( sz-- > 0 ) {
    str += (char)net->get();
  }

  return true;
}

__DLLEXPORT
bool NetTransport_base::push( const Event& __rs, const Event::key_type& rmkey,
                              const Event::key_type& srckey, const Event::key_type& __sid )
{
//  cerr << "<<<<<<<-------------------------\n";
//  dump( std::cerr, __rs );
//  std::cerr << "Src key " << hex << srckey << ", remote key " << rmkey
//            << " SID: " << _sid << "\n";

  __stl_assert( net != 0 );
  unsigned buf[7];

  buf[0] = to_net( __rs.code() );
  buf[1] = to_net( /* __rs.dest() */ rmkey );
  buf[2] = to_net( /* __rs.src() */ srckey );
  buf[3] = to_net( /* __rs.sid() */ _sid ); // should be changed: that's another session!
  buf[4] = to_net( ++_count );
  buf[5] = to_net( __rs.responce() );
  buf[6] = to_net( __rs.value().size() );

  net->write( (const char *)buf, sizeof(unsigned) * 7 );

  copy( __rs.value().begin(), __rs.value().end(),
        ostream_iterator<char,char,char_traits<char> >(*net) );

  net->flush();

  return net->good();
}

__DLLEXPORT
void NetTransport::connect( sockstream& s, const string& hostname, string& info )
{
  cerr << "Connected: " << hostname << endl;

  Event ev;
  _sid = smgr.create();
  SessionInfo& sess = smgr[_sid];

  sess._host = hostname;
  sess._port = s.rdbuf()->port();
  net = &s;

  try {
//    if ( s.rdbuf()->stype() == sock_base::sock_stream ) {
      while ( pop( ev ) ) {
        event_process( ev, sess, hostname );
      }
//    } else if ( s.rdbuf()->stype() == sock_base::sock_dgram ) {
      // indeed here need more check: data of event
      // and another: message can be break, and other datagram can be
      // in the middle of message...
//      cerr << "#1" << endl;
//      s.rdbuf()->pubsync();
//      if ( (s.rdbuf()->in_avail() >= 7 * sizeof(unsigned)) && pop(ev) ) {
//        cerr << "#2" << endl;
//        event_process( ev, sess, hostname );
//        cerr << "#3" << endl;
//      }
//    }
  }
  catch ( ios_base::failure& e ) {
    cerr << "Post mortem: " << e.what() << endl;
  }
  catch ( std::runtime_error& e ) {
    cerr << e.what() << endl;
  }
  catch ( std::exception& e ) {
    cerr << e.what() << endl;
  }
  catch ( ... ) {
    cerr << "What?!" << endl;
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

  _partner_name = hostname;
  if ( net->good() ) {
    heap_type::iterator r;

    // forward register remote object with ID 0
    r = rar.insert( 
      heap_type::value_type( 0,
                             EventHandler::manager()->SubscribeRemote( this, 0, _partner_name ) ) ).first;
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
//  me._sid = EventHandler::_mgr->establish_session();
//  SessionInfo& sess =  EventHandler::_mgr->session_info( _sid );

  try {
    while ( me.pop( ev ) ) {
      // dump( std::cerr, ev );
      __stl_assert( EventHandler::manager() != 0 );
      // if _mgr == 0, best choice is close connection...
//      sess.inc_from( sizeof(__Event_Base) + sizeof(std::string::size_type) +
//                     ev.value_size() );
//      if ( sess.un_from( ev.seq() ) != 0 ) {
//        cerr << "Event(s) lost, or miss range event" << endl;
//      }
      r = me.rar.find( ev._src );
      if ( r == me.rar.end() ) {
        r = me.rar.insert( 
          heap_type::value_type( ev._src,
          EventHandler::manager()->SubscribeRemote( &me, ev._src, me._partner_name ) ) ).first;
//        cerr << "Create remote object: " << hex << (*r).second
//             << " [" << ev._src << "]\n";
      }
      ev._src = (*r).second; // substitute my local id
      if ( me._sid == -1 ) {
        me._sid = ev.sid();
      }

      EventHandler::manager()->Dispatch( ev );
    }
  }
  catch ( ios_base::failure& e ) {
    cerr << "Post mortem: " << e.what() << endl;
  }
  catch ( std::runtime_error& e ) {
    cerr << e.what() << endl;
  }
  catch ( std::exception& e ) {
    cerr << e.what() << endl;
  }
  catch ( ... ) {
    cerr << "What?!" << endl;
  }

  return 0;  
}

__DLLEXPORT
void NetTransportMP::connect( sockstream& s, const string& hostname, string& info )
{
  cerr << "Connected: " << hostname << endl;

  Event ev;
  _sid = smgr.create();
  SessionInfo& sess = smgr[ _sid ];

  sess._host = hostname;
  sess._port = s.rdbuf()->port();
  net = &s;

  try {
    // indeed here need more check: data of event
    // and another: message can be break, and other datagram can be
    // in the middle of message...
    if ( pop( ev ) ) {
      event_process( ev, sess, hostname );
    }
  }
  catch ( ios_base::failure& e ) {
    cerr << "Post mortem: " << e.what() << endl;
  }
  catch ( std::runtime_error& e ) {
    cerr << e.what() << endl;
  }
  catch ( std::exception& e ) {
    cerr << e.what() << endl;
  }
  catch ( ... ) {
    cerr << "What?!" << endl;
  }
  cerr << "Disconnected: " << hostname << endl;
}

} // namespace EDS

// -*- C++ -*- Time-stamp: <99/03/26 14:35:38 ptr>

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

__DLLEXPORT NetTransport::~NetTransport()
{
  if ( _net_owner ) {
    delete net;
  }
  EventHandler::_mgr->Remove( this );
}

int NetTransport::_loop( void *p )
{
  NetTransport& me = *reinterpret_cast<NetTransport *>(p);
  heap_type::iterator r;
  Event ev;

  try {
    while ( me.pop(ev) ) {
      // dump( std::cerr, ev );
      __stl_assert( EventHandler::_mgr != 0 );
      // if _mgr == 0, best choice is close connection...
      r = me.rar.find( ev._src );
      if ( r == me.rar.end() ) {
        r = me.rar.insert( 
          heap_type::value_type( ev._src,
          EventHandler::_mgr->SubscribeRemote( &me, ev._src, me._partner_name ) ) ).first;
//        cerr << "Create remote object: " << hex << (*r).second
//             << " [" << ev._src << "]\n";
      }
      ev._src = (*r).second; // substitute my local id
      if ( me._sid == -1 ) {
        me._sid = ev.sid();
      }

      EventHandler::_mgr->Dispatch( ev );
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

// connect initiator (client) function
__DLLEXPORT
NetTransport::key_type NetTransport::open( const string& hostname, int port )
{
  if ( _net_owner && net != 0 ) {
    if ( net->is_open() ) {
      net->close();
    }
    net->open( hostname.c_str(), port );
  } else {
    net = new std::sockstream( hostname.c_str(), port );
  }
  _net_owner = true;
  _partner_name = hostname;
  if ( net->good() ) {
    heap_type::iterator r;

    // forward register remote object with ID 0
    r = rar.insert( 
      heap_type::value_type( 0,
                             EventHandler::_mgr->SubscribeRemote( this, 0, _partner_name ) ) ).first;
    _thr.launch( _loop, this ); // start thread here
    return (*r).second;
  }
  return -1;
}

// passive side (server) function

__DLLEXPORT
void NetTransport::connect( sockstream& s, const string& hostname, string& info )
{
  cerr << "Connected: " << hostname << endl;

  heap_type::iterator r;
  Event ev;
  _sid = EventHandler::_mgr->establish_session();
  SessionInfo& sess =  EventHandler::_mgr->session_info( _sid );

  try {
    net = &s;
    while ( pop(ev) ) {
//      cerr << "------------------------>>>>>>>>\n";
//      dump( std::cerr, ev );
      __stl_assert( EventHandler::_mgr != 0 );
      // if _mgr == 0, best choice is close connection...
      sess.inc_from( sizeof(__Event_Base) + sizeof(std::string::size_type) +
                     ev.value_size() );
      if ( sess.un_from( ev.seq() ) != 0 ) {
        cerr << "Event(s) lost, or miss range event" << endl;
      }
      r = rar.find( ev._src );
      if ( r == rar.end() ) {        
        r = rar.insert( 
          heap_type::value_type( ev._src,
          EventHandler::_mgr->SubscribeRemote( this, ev._src, hostname ) ) ).first;
//        cerr << "Create remote object: " << hex << (*r).second
//             << " [" << ev._src << "]\n";
      }
      ev._src = (*r).second; // substitute my local id

      EventHandler::_mgr->Dispatch( ev );
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

bool NetTransport::pop( Event& __rs )
{
  unsigned buf[7];

  net->read( (char *)&buf, sizeof(int) * 7 );
  __rs.code( from_net( buf[0] ) );
  __rs.dest( from_net( buf[1] ) );
  __rs.src( from_net( buf[2] ) );
  __rs.sid( from_net( buf[3] ) );
  __rs.seq( from_net( buf[4] ) );
  __rs.responce( from_net( buf[5] ) );

  unsigned sz = from_net( buf[6] );
  string& str = __rs.value();

  if ( !net->good() ) {
    return false;
  }
  str.erase();  // str.clear(); absent in VC's STL
  str.reserve( sz );
  while ( sz-- > 0 ) {
    str += (char)net->get();
  }

  return true;
}

__DLLEXPORT
bool NetTransport::push( const Event& __rs, const Event::key_type& rmkey,
                         const Event::key_type& srckey )
{
//  cerr << "<<<<<<<-------------------------\n";
//  dump( std::cerr, __rs );
//  std::cerr << "Src key " << hex << srckey << ", remote key " << rmkey
//            << " SID: " << _sid << "\n";

  unsigned buf[7];

  buf[0] = to_net( __rs.code() );
  buf[1] = to_net( /* __rs.dest() */ rmkey );
  buf[2] = to_net( /* __rs.src() */ srckey );
  buf[3] = to_net( /* __rs.sid() */ _sid );
  buf[4] = to_net( ++_count );
  buf[5] = to_net( __rs.responce() );
  buf[6] = to_net( __rs.value().size() );

  net->write( (const char *)buf, sizeof(unsigned) * 7 );

  copy( __rs.value().begin(), __rs.value().end(),
        ostream_iterator<char,char,char_traits<char> >(*net) );

  net->flush();

  return net->good();
}

} // namespace EDS

// -*- C++ -*- Time-stamp: <99/03/22 12:12:35 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <iomanip>
#include <NetTransport.h>
#include <EventHandler.h>
#include <EvManager.h>


namespace EDS {

using namespace std;

__DLLEXPORT
void dump( ostream& o, const EDS::Event& e )
{
  o << setiosflags(ios_base::showbase) << hex
    << "Code: " << e.code() << " Destination: " << e.dest() << " Source: " << e.src()
    << "\nSeq: " << e.seq()
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

int NetTransport::_loop( void *p )
{
  NetTransport& me = *reinterpret_cast<NetTransport *>(p);
  heap_type::iterator r;
  Event ev;

  try {
    while ( me.pop(ev) ) {
      dump( std::cerr, ev );
      __stl_assert( EventHandler::_mgr != 0 );
      // if _mgr == 0, best choice is close connection...
      r = me.rar.find( ev._src );
      if ( r == me.rar.end() ) {        
        r = me.rar.insert( 
          heap_type::value_type( ev._src,
          EventHandler::_mgr->SubscribeRemote( &me, ev._src, me._server_name ) ) ).first;
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
  _server_name = hostname;
  if ( net->good() ) {
    heap_type::iterator r;

    // forward register remote object with ID 0
    r = rar.insert( 
      heap_type::value_type( 0,
                             EventHandler::_mgr->SubscribeRemote( this, 0, _server_name ) ) ).first;
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

  try {
    net = &s;
    while ( pop(ev) ) {
      dump( std::cerr, ev );
      __stl_assert( EventHandler::_mgr != 0 );
      // if _mgr == 0, best choice is close connection...
      r = rar.find( ev._src );
      if ( r == rar.end() ) {        
        r = rar.insert( 
          heap_type::value_type( ev._src,
          EventHandler::_mgr->SubscribeRemote( this, ev._src, hostname ) ) ).first;
        cerr << "Create remote object: " << hex << (*r).second
             << " [" << ev._src << "]\n";
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
  unsigned buf[5];

  net->read( (char *)&buf, sizeof(int) * 5 );
  __rs.code( from_net( buf[0] ) );
  __rs.dest( from_net( buf[1] ) );
  __rs.src( from_net( buf[2] ) );
  __rs.seq( from_net( buf[3] ) );

  unsigned sz = from_net( buf[4] );
  string& str = __rs.value();

  if ( !net->good() ) {
    return false;
  }
  str.clear();
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
  dump( std::cerr, __rs );
  std::cerr << "Src key " << hex << srckey << ", remote key " << rmkey
            << "\n";

  unsigned buf[5];

  buf[0] = to_net( __rs.code() );
  buf[1] = to_net( /* __rs.dest() */ rmkey );
  buf[2] = to_net( /* __rs.src() */ srckey );
  buf[3] = to_net( __rs.seq() != 0 ? __rs.seq() | Event::respbit : ++_count );
  buf[4] = to_net( __rs.value().size() );

  net->write( (const char *)buf, sizeof(unsigned) * 5 );

  copy( __rs.value().begin(), __rs.value().end(),
        ostream_iterator<char,char,char_traits<char> >(*net) );

  net->flush();

  return net->good();
}

} // namespace EDS

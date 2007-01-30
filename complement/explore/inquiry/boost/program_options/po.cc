// -*- C++ -*- Time-stamp: <07/01/25 20:52:45 ptr>

#include <iostream>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

int main( int argc, const char **argv )
{
  int port;
  bool is_daemon = false;
  std::string catalog;
  try {
    po::options_description op( "Copyright (C) RightMedia, 2006-2007\nThis is intercessor to reportware (http request retranslator, storing answers)\nOptions" );
    op.add_options()
      ( "help,h", "print this help message" )
      ( "port,p", po::value<int>(&port)->default_value(8080), "listen port (default 8080)" )
      ( "save,s", po::value<string>(&catalog), "store uploaded stream to catalog (default none)" )
      ( "daemon,d", "become daemon" );
    
    po::variables_map vm;
    po::store( po::parse_command_line( argc, const_cast<char **>(argv), op ), vm );
    po::notify( vm );

    if ( vm.count( "help" ) ) {
      cerr << op << endl;
      return 0;
    }

    // if ( vm.count( "port" ) ) {
    //   port = vm["port"].as<int>();
    // }

    // if ( vm.count( "save" ) ) {
    //   catalog = vm["save"].as<string>();
    // }

    if ( vm.count( "daemon" ) ) {
      is_daemon = true;
    }

    cout << "Port: " << port << "\n"
         << "Catalog: " << catalog << "\n"
         << "Daemon: " << is_daemon << endl;
  }
  catch ( int i ) {
    cerr << "exception i " << i << endl;
  }
  catch ( exception& err ) {
    cerr << "exception " << err.what() << endl;
  }

  return 0;
}

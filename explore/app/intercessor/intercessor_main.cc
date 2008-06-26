// -*- C++ -*- Time-stamp: <07/03/07 16:37:34 ptr>

#include <iostream>
#include <stdexcept>

#include <mt/xmt.h>
#include <sockios/sockmgr.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

#include "server.h"
#include "intercessor.h"

using namespace std;
namespace po = boost::program_options;

boost::filesystem::path dir( boost::filesystem::initial_path() );

unsigned rq_timeout;

int main( int argc, char * const * argv )
{
  int port;
  bool is_daemon = false;
  std::string catalog;

  try {
    po::options_description op( "This is intercessor to reportware (http request retranslator, storing answers)\nOptions" );
    op.add_options()
      ( "help,h", "print this help message" )
      ( "port,p", po::value<int>(&port)->default_value(8080), "listen port (default 8080)" )
      ( "save,s", po::value<string>(&catalog), "store uploaded stream to files in this catalog (default none)" )
      ( "timeout,t", po::value<unsigned>(&rq_timeout)->default_value(3), "timeout for response from ReportWare Server, seconds (default 3)" )
      ( "daemon,d", "become daemon" )
      ;
    
    po::variables_map vm;
    po::store( po::parse_command_line( argc, const_cast<char **>(argv), op ), vm );
    po::notify( vm );

    if ( vm.count( "help" ) ) {
      cerr << op << endl;
      return 0;
    }

    if ( vm.count( "daemon" ) ) {
      is_daemon = true;
    }

    namespace fs = boost::filesystem;

    dir = fs::system_complete( fs::path( catalog, fs::native ) );

    if ( !fs::exists( dir ) ) {
      throw runtime_error( string( "Catalog " ) + catalog + " not exists" );
    }

    if ( !fs::is_directory( dir ) ) {
      throw runtime_error( string( "Not a catalog: " ) + catalog );
    }
  }
  catch ( runtime_error& err ) {
    cerr << "Error: " << err.what() << endl;
    return -1;
  }
  catch ( exception& err ) {
    cerr << "Error: " << err.what() << endl;
    return -1;
  }
  catch ( int r ) {
    return r;
  }

  try {
    if ( is_daemon ) {
      xmt::become_daemon();
    }
    intr::Intercessor intercessor(0);

    sockmgr_stream_MP<intr::IncomeHttpRqProcessor> mgr( port );

    mgr.wait();
  }
  catch ( xmt::fork_in_parent& child ) {
    // child.pid();
  }
  catch ( std::runtime_error& err ) {
    cerr << err.what() << endl;
  }
  catch ( ... ) {
  }

  return 0;
}

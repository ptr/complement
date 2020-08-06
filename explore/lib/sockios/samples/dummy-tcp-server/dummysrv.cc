// -*- C++ -*-

/*
 *
 * Copyright (c) 2020
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <misc/opts.h>
#include <sockios/sockstream>
#include <sockios/socksrv.h>
#include <thread>

#include <iostream>
#include <iomanip>
#include <string>

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include <fstream>

#include <mt/fork.h>

using namespace std;

class Dummy
{
  public:
    Dummy(sockstream&);
    
    ~Dummy();

    void connect(sockstream& s);
};

Dummy::Dummy(sockstream&)
{
  // on new connection
  cerr << "On connect" << endl;
}

Dummy::~Dummy()
{
  // on close connection
  cerr << "On close" << endl;
}

void Dummy::connect(sockstream& s)
{
  cerr << "Read ";
  copy(istreambuf_iterator<char>(s), istreambuf_iterator<char>(), ostreambuf_iterator<char>(cerr));
  cerr << endl;
}

int main( int argc, const char **argv )
{
  Opts opts;

  opts.description( "dummy server" );
  opts.usage( "[options]" );

  opts << option<void>( "print this help message", 'h', "help" )
       << option<short>( "listen <port>", 'p', "port" )[4385]
       << option<void>( "become daemon", 'd', "detach" )
       << option<string>( "write pid to <file>, if run as daemon", "pidfile" )["/var/run/dummysrv.pid"]
       << option<unsigned>( "set uid, if run as daemon", 'u', "uid" )
       << option<unsigned>( "set gid, if run as daemon", 'g', "gid" );

  try {
    opts.parse( argc, argv );
  }
  catch (...) {
    opts.help( cerr );
    return 1;
  }

  if ( opts.is_set( 'h' ) ) {
    opts.help( cout );
    return 0;
  }

  try {
    if ( opts.is_set( 'd' ) ) {
      std::tr2::this_thread::become_daemon();
    }


    std::tr2::this_thread::block_signal( SIGINT );
    std::tr2::this_thread::block_signal( SIGQUIT );
    std::tr2::this_thread::block_signal( SIGILL );
    std::tr2::this_thread::block_signal( SIGABRT );
    std::tr2::this_thread::block_signal( SIGFPE );
    std::tr2::this_thread::block_signal( SIGSEGV );
    std::tr2::this_thread::block_signal( SIGTERM );
    std::tr2::this_thread::signal_handler( SIGPIPE, SIG_IGN );

    short port = opts.get<short>( 'p' );
    connect_processor<Dummy> srv(port);

    if ( !srv.is_open() ) {
      return 1;
    }

    if ( opts.is_set( 'd' ) ) {
      if ( opts.is_set( 'g' ) ) {
        gid_t gid = opts.get<unsigned>( 'g' );
        setgid( gid );
      }
      if ( opts.is_set( 'u' ) ) {
        uid_t uid = opts.get<unsigned>( 'u' );
        setuid( uid );
      }
    }

    sigset_t signal_mask;

    sigemptyset( &signal_mask );
    sigaddset( &signal_mask, SIGINT );
    sigaddset( &signal_mask, SIGQUIT );
    sigaddset( &signal_mask, SIGILL );
    sigaddset( &signal_mask, SIGABRT );
    sigaddset( &signal_mask, SIGFPE );
    sigaddset( &signal_mask, SIGSEGV );
    sigaddset( &signal_mask, SIGTERM );

    int sig_caught;
    sigwait( &signal_mask, &sig_caught );

    switch ( sig_caught ) {
      case SIGINT:
      case SIGQUIT:
      case SIGILL:
      case SIGABRT:
      case SIGFPE:
      case SIGKILL:
      case SIGSEGV:
      case SIGTERM:
        srv.close();
        srv.wait();
        break;
    }

    switch ( sig_caught ) {
      case SIGABRT:
      case SIGFPE:
      case SIGSEGV:
        return sig_caught;
    }
  }
  catch( const runtime_error& e) {
    cout << e.what() << endl;
  }
  catch ( tr2::fork_in_parent& child ) {
    ofstream pidfile( opts.get<string>( "pidfile" ).c_str() );

    pidfile << child.pid();
  }

  return 0;
}

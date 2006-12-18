// -*- C++ -*- Time-stamp: <06/12/18 18:51:17 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_test.h"
#include "message.h"

#include <boost/test/unit_test.hpp>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <list>

#include <arpa/inet.h>

#include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>

using namespace boost::unit_test_framework;
using namespace std;

void sockios_test::hostname_test()
{
  unsigned long local = htonl( 0x7f000001 ); // 127.0.0.1

#ifdef _LITTLE_ENDIAN
  BOOST_CHECK_EQUAL( local, 0x0100007f );
#endif

#ifdef _BIG_ENDIAN
  BOOST_CHECK_EQUAL( local, 0x7f000001 );
#endif

  BOOST_CHECK_EQUAL( std::hostname( local ), "localhost [127.0.0.1]" );

#ifdef __unix
  char buff[1024];

  gethostname( buff, 1024 );

  BOOST_CHECK_EQUAL( std::hostname(), buff );
#endif
}

void sockios_test::service_test()
{
#ifdef __unix
  BOOST_CHECK( std::service( "ftp", "tcp" ) == 21 );
  BOOST_CHECK( std::service( 7, "udp" ) == "echo" );
#else
  BOOST_ERROR( "requests for service (/etc/services) not implemented on this platform" );
#endif
}

void sockios_test::hostaddr_test1()
{
#ifdef __unix
  in_addr addr = std::findhost( "localhost" );

# ifdef _LITTLE_ENDIAN
  BOOST_CHECK_EQUAL( addr.s_addr, 0x0100007f );
# endif

# ifdef _BIG_ENDIAN
  BOOST_CHECK_EQUAL( addr.s_addr, 0x7f000001 );
# endif
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

void sockios_test::hostaddr_test2()
{
#ifdef __unix
  list<in_addr> haddrs;
  std::gethostaddr( "localhost", back_inserter(haddrs) );

  bool localhost_found = false;

  for ( list<in_addr>::const_iterator i = haddrs.begin(); i != haddrs.end(); ++i ) {
    if ( i->s_addr == htonl( 0x7f000001 ) ) { // 127.0.0.1
      localhost_found = true;
      break;
    }
  }
  
  BOOST_CHECK( localhost_found == true );
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

void sockios_test::hostaddr_test3()
{
#ifdef __unix
  list<sockaddr> haddrs;
  gethostaddr2( "localhost", back_inserter(haddrs) );

  bool localhost_found = false;

  for ( list<sockaddr>::const_iterator i = haddrs.begin(); i != haddrs.end(); ++i ) {
    switch ( i->sa_family ) {
      case PF_INET:
        if ( ((sockaddr_in *)&*i)->sin_addr.s_addr == htonl( 0x7f000001 ) ) {
          localhost_found = true;
        }
        break;
      case PF_INET6:
        if ( ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[0] == 0 &&
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[1] == 0 && 
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[2] == 0 &&
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[3] == 1 ) {
          localhost_found = true;
        }
        break;
    }
  }
  
  BOOST_CHECK( localhost_found == true );
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

class Cnt
{
  public:
    Cnt( sockstream& )
      { xmt::Locker lk(lock); ++cnt; }

    ~Cnt()
      { xmt::Locker lk(lock); --cnt; }

    void connect( sockstream& )
      { }

    void close()
      { }

    static xmt::Mutex lock;
    static int cnt;
};

xmt::Mutex Cnt::lock;
int Cnt::cnt = 0;

void sockios_test::ctor_dtor()
{
  // Check, that naumber of ctors of Cnt is the same as number of called dtors
  // i.e. all created Cnt freed.
  // due to async nature of communication, no way to check Cnt::cnt
  // before server stop.
  {
    sockmgr_stream_MP<Cnt> srv( port );

    {
      sockstream s1( "localhost", port );

      BOOST_CHECK( s1.good() );
      BOOST_CHECK( s1.is_open() );

      s1 << "1234" << endl;

      BOOST_CHECK( s1.good() );
      BOOST_CHECK( s1.is_open() );
    }

    srv.close();
    srv.wait();

    Cnt::lock.lock();
    BOOST_CHECK( Cnt::cnt == 0 );
    Cnt::lock.unlock();
    
  }
  {
    sockmgr_stream_MP<Cnt> srv( port );

    {
      sockstream s1( "localhost", port );
      sockstream s2( "localhost", port );

      BOOST_CHECK( s1.good() );
      BOOST_CHECK( s1.is_open() );
      BOOST_CHECK( s2.good() );
      BOOST_CHECK( s2.is_open() );

      s1 << "1234" << endl;
      s2 << "1234" << endl;

      BOOST_CHECK( s1.good() );
      BOOST_CHECK( s1.is_open() );
      BOOST_CHECK( s2.good() );
      BOOST_CHECK( s2.is_open() );
    }

    srv.close();
    srv.wait();

    Cnt::lock.lock();
    BOOST_CHECK( Cnt::cnt == 0 );
    Cnt::lock.unlock();
    
  }
}

class loader
{
  public:
    loader( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

loader::loader( std::sockstream& )
{
}

void loader::connect( std::sockstream& s )
{
  char buf[1024];

  s.read( buf, 1024 );
  s.write( buf, 1024 );
  s.flush();
}

void loader::close()
{
}

xmt::Thread::ret_code client_thr( void * )
{
  xmt::Thread::ret_code ret;

  ret.iword = 0;

  sockstream s( "localhost", port );

  char buf[1024];

  fill( buf, buf + 1024, 0 );

  while( true ) {
    s.write( buf, 1024 );
    s.flush();

    s.read( buf, 1024 );
  }

  return ret;
}

void sigpipe_handler( int sig, siginfo_t *si, void * )
{
  cerr << "-----------------------------------------------\n"
       << "my pid: " << xmt::getpid() << ", ppid: " << xmt::getppid() << "\n"
       << "signal: " << sig << ", number " << si->si_signo
       << " errno " << si->si_errno
       << " code " << si->si_code << endl;
}
 
void sockios_test::sigpipe()
{
  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  BOOST_REQUIRE( id != -1 );
  // if ( id == -1 ) {
  //   cerr << "Error on shmget" << endl;
  // }
  BOOST_REQUIRE( shmctl( id, IPC_STAT, &ds ) != -1 );
  // if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
  //   cerr << "Error on shmctl" << endl;
  // }
  void *buf = shmat( id, 0, 0 );
  BOOST_REQUIRE( buf != reinterpret_cast<void *>(-1) );
  // if ( buf == reinterpret_cast<void *>(-1) ) {
  //   cerr << "Error on shmat" << endl;
  // }

  xmt::__Condition<true>& fcnd = *new( buf ) xmt::__Condition<true>();
  fcnd.set( false );
  xmt::__Condition<true>& tcnd = *new( (char *)buf + sizeof(xmt::__Condition<true>) ) xmt::__Condition<true>();
  tcnd.set( false );

  try {
    xmt::fork();

    fcnd.try_wait();

    try {
      /*
       * This process will be killed,
       * so I don't care about safe termination.
       */
      xmt::Thread *th1 = new xmt::Thread( client_thr );
      for ( int i = 0; i < 10; ++i ) {
        new xmt::Thread( client_thr );
        new xmt::Thread( client_thr );
        new xmt::Thread( client_thr );
        new xmt::Thread( client_thr );
      }

      xmt::delay( xmt::timespec(5,0) );

      tcnd.set( true );

      th1->join();

      exit( 0 );
    }
    catch ( ... ) {
    }
  }
  catch ( xmt::fork_in_parent& child ) {
    try {
      xmt::signal_handler( SIGPIPE, &sigpipe_handler );
      sockmgr_stream_MP<loader> srv( port );

      fcnd.set( true );

      tcnd.try_wait();

      kill( child.pid(), SIGTERM );

      int stat;
      waitpid( child.pid(), &stat, 0 );

      srv.close();
      srv.wait();
    }
    catch ( ... ) {
    }
  }

  (&fcnd)->~__Condition<true>();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds ); 
}

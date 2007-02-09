// -*- C++ -*- Time-stamp: <07/02/07 10:28:34 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_test.h"

#include <boost/test/unit_test.hpp>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <list>

#include <arpa/inet.h>

using namespace boost::unit_test_framework;
using namespace std;

/* ************************************************************ */

void names_sockios_test::hostname_test()
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

/* ************************************************************ */

void names_sockios_test::service_test()
{
#ifdef __unix
  BOOST_CHECK( std::service( "ftp", "tcp" ) == 21 );
  BOOST_CHECK( std::service( 7, "udp" ) == "echo" );
#else
  BOOST_ERROR( "requests for service (/etc/services) not implemented on this platform" );
#endif
}

/* ************************************************************ */

void names_sockios_test::hostaddr_test1()
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

/* ************************************************************ */

void names_sockios_test::hostaddr_test2()
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

/* ************************************************************ */

void names_sockios_test::hostaddr_test3()
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

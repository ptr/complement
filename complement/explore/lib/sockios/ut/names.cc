// -*- C++ -*- Time-stamp: <08/03/27 11:04:23 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_test.h"

#include <exam/suite.h>

#include <sockios/netinfo.h>

#include <list>

using namespace std;

/* ************************************************************ */

int EXAM_IMPL(names_sockios_test::hostname_test)
{
  unsigned long local = htonl( 0x7f000001 ); // 127.0.0.1

#ifdef _LITTLE_ENDIAN
  EXAM_CHECK( local == 0x0100007f );
#endif

#ifdef _BIG_ENDIAN
  EXAM_CHECK( local == 0x7f000001 );
#endif

  EXAM_CHECK( std::hostname( local ) == "localhost [127.0.0.1]" );

#ifdef __unix
  char buff[1024];

  gethostname( buff, 1024 );

  EXAM_CHECK( std::hostname() == buff );
#endif

  return EXAM_RESULT;
}

/* ************************************************************ */

int EXAM_IMPL(names_sockios_test::service_test)
{
#ifdef __unix
  EXAM_CHECK( std::service( "ftp", "tcp" ) == 21 );
  EXAM_CHECK( std::service( 7, "udp" ) == "echo" );
#else
  BOOST_ERROR( "requests for service (/etc/services) not implemented on this platform" );
#endif

  return EXAM_RESULT;
}

/* ************************************************************ */

int EXAM_IMPL(names_sockios_test::hostaddr_test1)
{
#ifdef __unix
  in_addr addr = std::findhost( "localhost" );

# ifdef _LITTLE_ENDIAN
  EXAM_CHECK( addr.s_addr == 0x0100007f );
# endif

# ifdef _BIG_ENDIAN
  EXAM_CHECK( addr.s_addr == 0x7f000001 );
# endif
  
#else
  EXAM_ERROR( "Not implemented" );
#endif

  return EXAM_RESULT;
}

/* ************************************************************ */

int EXAM_IMPL(names_sockios_test::hostaddr_test2)
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
  
  EXAM_CHECK( localhost_found == true );
  
#else
  EXAM_ERROR( "Not implemented" );
#endif

  return EXAM_RESULT;
}

/* ************************************************************ */

int EXAM_IMPL(names_sockios_test::hostaddr_test3)
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
  
  EXAM_CHECK( localhost_found == true );
  
#else
  EXAM_ERROR( "Not implemented" );
#endif

  return EXAM_RESULT;
}

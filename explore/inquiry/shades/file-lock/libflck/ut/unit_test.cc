// -*- C++ -*- Time-stamp: <04/01/13 16:44:56 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <boost/test/unit_test.hpp>
#include "../flck.h"
#include <mt/xmt.h>

#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

using namespace boost::unit_test_framework;

using namespace __impl;
using namespace std;

Mutex m;
int cnt = 0;

int thread_func( void * )
{
  for ( int count = 0; count < 10; ++count ) {
    try {
      int fd = open( "/tmp/myfile", O_RDWR | O_CREAT | O_APPEND, 00666 );

      BOOST_CHECK( fd >= 0 );
      if ( fd < 0 ) {
        throw errno;
      }

      int check = flck( fd, _F_LCK_W );

      BOOST_CHECK( check == 0 );

      if ( check != 0 ) {
        throw check;
      }

      write( fd, "test string 1\n", 14 );
      write( fd, "test string 2\n", 14 );
      write( fd, "test string 3\n", 14 );

      m.lock();
      ++cnt;
      m.unlock();

      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;

      Thread::delay( &t );

      check = flck( fd, _F_UNLCK );

      BOOST_CHECK( check == 0 );

      if ( check != 0 ) {
        throw check;
      }

      close( fd );
    }
    catch ( int check ) {
      cerr << "The error returned: " << check << ", errno " << errno << endl;
    }
  }

  return 0;
}

int thread_func_r( void * )
{
  char buf[15];
  buf[14] = 0;

  for ( int count = 0; count < 10; ++count ) {
    try {
      m.lock();
      int ready_file = cnt;
      m.unlock();

      if ( ready_file == 0 ) {
        --count;
        continue;
      }

      int fd = open( "/tmp/myfile", O_RDONLY );

      BOOST_CHECK( fd >= 0 );
      if ( fd < 0 ) {
        throw errno;
      }

      int check = flck( fd, _F_LCK_R );

      BOOST_CHECK( check == 0 );

      if ( check != 0 ) {
        throw check;
      }

      m.lock();
      int ready_cnt = cnt;
      m.unlock();

      int cmp;

      while ( ready_cnt-- > 0 ) {
        read( fd, buf, 14 );
        cmp = strcmp( buf, "test string 1\n" );
        BOOST_CHECK( cmp == 0 );
        if ( cmp != 0 ) {
          throw 0;
        }
        read( fd, buf, 14 );
        cmp = strcmp( buf, "test string 2\n" );
        BOOST_CHECK( cmp == 0 );
        if ( cmp != 0 ) {
          throw 0;
        }
        read( fd, buf, 14 );
        cmp = strcmp( buf, "test string 3\n" );
        BOOST_CHECK( cmp == 0 );
        if ( cmp != 0 ) {
          throw 0;
        }
      }

      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;

      Thread::delay( &t );

      check = flck( fd, _F_UNLCK );

      BOOST_CHECK( check == 0 );

      if ( check != 0 ) {
        throw check;
      }

      close( fd );
    }
    catch ( int check ) {
      cerr << "* The error returned: " << check << ", errno " << errno
           << buf << endl;
    }
  }

  return 0;
}

void flock_test()
{
  Thread t1( thread_func );
  Thread t2( thread_func );
  Thread t3( thread_func_r );
  Thread t4( thread_func_r );

  t1.join();
  t2.join();
  t3.join();
  t4.join();
}


#ifdef WIN32
test_suite *__cdecl init_unit_test_suite( int argc, char * * const argv )
#else
test_suite *init_unit_test_suite( int argc, char * * const argv )
#endif /* !WIN32 */
{
  test_suite *ts = BOOST_TEST_SUITE( "libflck test" );

  ts->add( BOOST_TEST_CASE( &flock_test ) );

  return ts;
}

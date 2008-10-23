// -*- C++ -*- Time-stamp: <04/01/12 15:32:54 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <iostream>

// #include <cstdio>
#include <sys/file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include <errno.h>

#include <mt/xmt.h>

using namespace std;
using namespace __impl;

// #define MARK cerr << __FILE__ << ":" << __LINE__ << endl
#define MARK (void *)0

// #define MARK fprintf( stdout, "%s:%d\n", __FILE__, __LINE__ )

int thread_func( void * )
{
  MARK;
  for ( int count = 0; count < 10; ++count ) {
    try {
      MARK;
      int fd = open( "/tmp/myfile", O_RDWR | O_CREAT | O_APPEND, 00666 );

      MARK;
      if ( fd < 0 ) {
        throw errno;
      }

      MARK;
      
      int check = flock( fd, LOCK_EX );
      if ( check != 0 ) {
        throw check;
      }

      MARK;
      write( fd, "test string 1\n", 14 );
      write( fd, "test string 2\n", 14 );
      write( fd, "test string 3\n", 14 );
      MARK;

      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;

      MARK;
      Thread::delay( &t );
      MARK;

      check = flock( fd, LOCK_UN );

      if ( check != 0 ) {
        throw check;
      }

      MARK;
      close( fd );
      MARK;
    }
    catch ( int check ) {
      cerr << "The error returned: " << check << ", errno " << errno << endl;
      MARK;
    }
  }

  return 0;
}

int main( int, char * const * )
{
  MARK;
  Thread t1( thread_func );
  MARK;
  Thread t2( thread_func );

  MARK;
  t2.join();
  t1.join();
  MARK;

  return 0;
}

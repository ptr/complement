// -*- C++ -*- Time-stamp: <04/01/09 14:17:50 ptr>

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

#include <mt/xmt.h>

using namespace std;
using namespace __impl;

int thread_func( void * )
{
  for ( int count = 0; count < 100; ++count ) {
    try {
      cerr << "1" << endl;
      int fd = open( "myfile", O_RDWR | O_CREAT | O_APPEND, 00666 );

      int check = flock( fd, LOCK_EX );
      if ( check != 0 ) {
        throw check;
      }

      write( fd, "test string 1\n", 14 );
      write( fd, "test string 2\n", 14 );
      write( fd, "test string 3\n", 14 );

      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;

      Thread::delay( &t );

      check = flock( fd, LOCK_UN );

      if ( check != 0 ) {
        throw check;
      }

      close( fd );
      cerr << "2" << endl;
    }
    catch ( int check ) {
      cerr << "The error returned: " << check << endl;
    }
  }

  return 0;
}

int main( int, char * const * )
{
  Thread t1( thread_func );
  Thread t2( thread_func );

  t2.join();
  t1.join();

  return 0;
}

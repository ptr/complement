// -*- C++ -*- Time-stamp: <04/01/12 15:14:22 ptr>

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

#include <fcntl.h>

using namespace std;
using namespace __impl;

#define MARK cerr << __FILE__ << ":" << __LINE__ << endl
// #define MARK (void *)0

// #define MARK fprintf( stdout, "%s:%d\n", __FILE__, __LINE__ )

// Condition cnd;

struct flock fl;
struct flock ful;

int thread_func1( void * )
{
  MARK;
  for ( int count = 0; count < 1; ++count ) {
    try {
      MARK;
      int fd = open( "/tmp/myfile", O_RDWR | O_CREAT | O_APPEND, 00666 );

      MARK;
      if ( fd < 0 ) {
        throw errno;
      }

      MARK;
      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 500000000;
//      Thread::delay( &t );
      
      // int check = flock( fd, LOCK_EX );
      int check = fcntl( fd, F_SETLKW, &fl );
      // int check = 0;
      if ( check != 0 ) {
        throw check;
      }
//      cnd.set( true );
      // pthread_yield();

      MARK;
      write( fd, "test string 1\n", 14 );
      write( fd, "test string 2\n", 14 );
      write( fd, "test string 3\n", 14 );
      MARK;

      // timespec t;
//      t.tv_sec = 1;
//      t.tv_nsec = 0;

      MARK;
      Thread::delay( &t );
//      for ( int i = 0; i < 0xfffff; ++i ) {
//        int x = 1;
//        int y = 2;
//        swap( x, y );
//      }
      MARK;

//      cnd.set( false );
      // check = flock( fd, LOCK_UN );
      check = fcntl( fd, F_SETLKW, &ful );

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

//  cerr << "-------------------------------" << endl;
//  cnd.set( true );
  return 0;
}

int thread_func2( void * )
{
  MARK;
  for ( int count = 0; count < 5; ++count ) {
    try {
      MARK;
      int fd = open( "/tmp/myfile", O_RDWR | O_CREAT | O_APPEND, 00666 );

      MARK;
      if ( fd < 0 ) {
        throw errno;
      }

      MARK;
//      cnd.try_wait();
      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;
      // Thread::delay( &t );
      // int check = flock( fd, LOCK_EX );
      int check = fcntl( fd, F_SETLKW, &fl );
      // int check = 0;
      if ( check != 0 ) {
        throw check;
      }

      MARK;
      write( fd, "test string 1\n", 14 );
      write( fd, "test string 2\n", 14 );
      write( fd, "test string 3\n", 14 );
      MARK;

      // timespec t;
//      t.tv_sec = 1;
//      t.tv_nsec = 0;

      MARK;
      Thread::delay( &t );
//      for ( int i = 0; i < 0xfffff; ++i ) {
//        int x = 1;
//        int y = 2;
//        swap( x, y );
//      }
      MARK;

      // check = flock( fd, LOCK_UN );
      check = fcntl( fd, F_SETLKW, &ful );

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
  fl.l_type = F_WRLCK;
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  fl.l_pid = getpid();

  ful.l_type = F_UNLCK;
  ful.l_start = 0;
  ful.l_whence = SEEK_SET;
  ful.l_len = 0;
  ful.l_pid = getpid();


  // cnd.set( false );
  // cnd.set( true );
  MARK;
  Thread t1( thread_func1 );
  MARK;
  Thread t2( thread_func2 );

  MARK;
  t2.join();
  t1.join();
  MARK;
//  cerr << "-------------------------------" << endl;
//  cnd.set( true );
  MARK;

  return 0;
}

// -*- C++ -*- Time-stamp: <04/01/12 16:46:26 ptr>

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

// #define MARK cerr << __FILE__ << ":" << __LINE__ << endl
#define MARK (void *)0

// #define MARK fprintf( stdout, "%s:%d\n", __FILE__, __LINE__ )

struct flock fl;
struct flock ful;
struct flock fls;

Mutex m;
int cnt = 0;

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
      
      int check = fcntl( fd, F_SETLKW, &fl );
      if ( check != 0 ) {
        throw check;
      }

      MARK;
      write( fd, "test string 1\n", 14 );
      write( fd, "test string 2\n", 14 );
      write( fd, "test string 3\n", 14 );

      m.lock();
      ++cnt;
      m.unlock();
      MARK;

      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;

      MARK;
      Thread::delay( &t );
      MARK;

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

int thread_func_r( void * )
{
  MARK;

  char buf[15];
  buf[14] = 0;

  for ( int count = 0; count < 10; ++count ) {
    try {
      MARK;
      int fd = open( "/tmp/myfile", O_RDONLY );

      MARK;
      if ( fd < 0 ) {
        throw errno;
      }

      MARK;
      
      int check = fcntl( fd, F_SETLKW, &fls );
      if ( check != 0 ) {
        throw check;
      }

      MARK;
      m.lock();
      int ready_cnt = cnt;
      m.unlock();
      
      while ( ready_cnt-- > 0 ) {
        read( fd, buf, 14 );
        if ( strcmp( buf, "test string 1\n" ) != 0 ) {
          throw 0;
        }
        read( fd, buf, 14 );
        if ( strcmp( buf, "test string 2\n" ) != 0 ) {
          throw 0;
        }
        read( fd, buf, 14 );
        if ( strcmp( buf, "test string 3\n" ) != 0 ) {
          throw 0;
        }
      }
      MARK;

      timespec t;
      t.tv_sec = 1;
      t.tv_nsec = 0;

      MARK;
      Thread::delay( &t );
      MARK;

      check = fcntl( fd, F_SETLKW, &ful );

      if ( check != 0 ) {
        throw check;
      }

      MARK;
      close( fd );
      MARK;
    }
    catch ( int check ) {
      cerr << "* The error returned: " << check << ", errno " << errno
           << buf << endl;
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

  fls.l_type = F_RDLCK;
  fls.l_start = 0;
  fls.l_whence = SEEK_SET;
  fls.l_len = 0;
  fls.l_pid = getpid();

  MARK;
  Thread t1( thread_func );
  MARK;
  Thread t2( thread_func );
  MARK;
  Thread t3( thread_func_r );
  MARK;
  Thread t4( thread_func_r );

  MARK;
  t2.join();
  t1.join();
  MARK;

  return 0;
}

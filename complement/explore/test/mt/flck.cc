// -*- C++ -*- Time-stamp: <07/07/16 21:34:09 ptr>

/*
 * Copyright (c) 2004, 2006, 2007
 * Petr Ovtchenkov
 *
 * Copyright (c) 2004
 * Kaspersky Labs
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/suite.h>

#include "mt/flck.h"
#include <mt/xmt.h>

#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

using namespace xmt;
using namespace std;

static const char *fname = "myfile";
static mutex m;
static mutex b;
static int cnt = 0;

static Thread::ret_code thread_func( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

  try {
    for ( int count = 0; count < 10; ++count ) {
      int fd = open( fname, O_RDWR | O_CREAT | O_APPEND, 00666 );

      b.lock();
      BOOST_CHECK( fd >= 0 );
      b.unlock();
      if ( fd < 0 ) {
        throw errno;
      }

      int check = flck( fd, _F_LCK_W );

      b.lock();
      BOOST_CHECK( check == 0 );
      b.unlock();

      if ( check != 0 ) {
        throw check;
      }

      write( fd, "test string 1\n", 14 );
      write( fd, "test string 2\n", 14 );
      write( fd, "test string 3\n", 14 );

      m.lock();
      ++cnt;
      m.unlock();

      delay( xmt::timespec(1,0) );

      check = flck( fd, _F_UNLCK );

      b.lock();
      BOOST_CHECK( check == 0 );
      b.unlock();

      if ( check != 0 ) {
        throw check;
      }

      close( fd );
    }
  }
  catch ( int check ) {
    b.lock();
    BOOST_MESSAGE( "* The error returned: " << check << ", errno " << errno );
    b.unlock();
    rt.iword = -1;
  }

  return rt;
}

static Thread::ret_code thread_func_r( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

  char buf[64];
  // buf[14] = 0;
  int ln = 0;

  try {
    int ready_rec;

    do {
      m.lock();
      ready_rec = cnt;
      m.unlock();
    } while( ready_rec == 0 );

    for ( int count = 0; count < 10; ++count ) {
      int fd = open( fname, O_RDONLY );

      b.lock();
      BOOST_CHECK( fd >= 0 );
      b.unlock();

      if ( fd < 0 ) {
        throw errno;
      }

      int check = flck( fd, _F_LCK_R );

      b.lock();
      BOOST_CHECK( check == 0 );
      b.unlock();

      if ( check != 0 ) {
        throw check;
      }

      m.lock();
      int ready_cnt = cnt;
      m.unlock();

      int cmp;

      while ( ready_cnt-- > 0 ) {
        memset( buf, 0, 64 );
        int v = read( fd, buf, 14 );
        while ( v < 14 ) {
          v += read( fd, buf + v, 14 - v );
        }
        ++ln;
        cmp = strcmp( buf, "test string 1\n" );
        b.lock();
        BOOST_CHECK( cmp == 0 );
        b.unlock();
        if ( cmp != 0 ) {
          throw 0;
        }
        memset( buf, 0, 64 );
        v = read( fd, buf, 14 );
        while ( v < 14 ) {
          v += read( fd, buf + v, 14 - v );
        }
        ++ln;
        cmp = strcmp( buf, "test string 2\n" );

        b.lock();
        BOOST_CHECK( cmp == 0 );
        b.unlock();

        if ( cmp != 0 ) {
          throw 0;
        }
        memset( buf, 0, 64 );
        v = read( fd, buf, 14 );
        while ( v < 14 ) {
          v += read( fd, buf + v, 14 - v );
        }
        ++ln;
        cmp = strcmp( buf, "test string 3\n" );
        b.lock();
        BOOST_CHECK( cmp == 0 );
        if ( cmp != 0 ) {
          cerr << "* The error returned: " << cmp << ", errno " << errno << ' ' << buf << " " << v << " " << ln << endl;
        }
        b.unlock();
        if ( cmp != 0 ) {
          throw 0;
        }
      }

      delay( xmt::timespec(1,0) );

      check = flck( fd, _F_UNLCK );

      b.lock();
      BOOST_CHECK( check == 0 );
      b.unlock();

      if ( check != 0 ) {
        throw check;
      }

      close( fd );
    }
  }
  catch ( int check ) {
    b.lock();
    BOOST_MESSAGE( "* The error returned: " << check << ", errno " << errno << ' ' << buf );
    b.unlock();
    rt.iword = -1;
  }

  return rt;
}

int EXAM_IMPL(flock_test)
{
  unlink( fname );
  Thread t1( thread_func );
  Thread t2( thread_func );
  Thread t3( thread_func_r );
  Thread t4( thread_func_r );

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  unlink( fname );

  return EXAM_RESULT;
}

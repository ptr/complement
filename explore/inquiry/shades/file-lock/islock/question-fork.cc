// -*- C++ -*- Time-stamp: <04/01/29 11:18:15 ptr>

/*
 *
 * Copyright (c) 2002, 2003
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#include <mt/xmt.h>
#include <iostream>
#include <fstream>
#include "../libflck/flck.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/file.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

using namespace std;
using namespace __impl;

static int count = 0;

static int thr_count = 0;

Mutex m;

int f( void * )
{
  int fd = open( "myfile", O_RDWR | O_CREAT | O_APPEND, 00666 );

  MT_LOCK( m );
  ofstream f( fd );
  f << ++thr_count << endl;
  f.close();
  MT_UNLOCK( m );

  for ( int i = 0; i < 100; ++i ) {
    int check = flck( fd, _F_LCK_W ) /* flock( fd, LOCK_EX ) */;
    if ( check != 0 ) {
      cerr << "fail" << endl;
      return -1;
    }
    MT_LOCK( m );
    ++::count;
    MT_UNLOCK( m );

    int j = 0;
    while ( j++ < 1000000 );

    MT_LOCK( m );
    --::count;

    if ( ::count != 0 ) {
      cerr << "count is " << ::count << endl;
    }
    MT_UNLOCK( m );
    check = flck( fd, _F_UNLCK ) /* flock( fd, LOCK_UN ) */;
    if ( check != 0 ) {
      cerr << "fail 2" << endl;
      return -1;
    }
  }

  ::close( fd );

  return 0;
}

int main()
{
  // Thread t[30];
  pid_t t[30];

  for ( int i = 0; i < 30; ++i ) {
    // t[i].launch( f );
    t[i] = fork();
    if ( t[i] == 0 ) {
      f(0);
      return 0;
    }
  }

  int status;
  for ( int i = 0; i < 30; ++i ) {
    waitpid( t[i], &status, 0 );
  }

  return 0;
}

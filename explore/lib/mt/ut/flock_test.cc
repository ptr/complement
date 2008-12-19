// -*- C++ -*- Time-stamp: <08/03/26 10:08:36 ptr>

/*
 * Copyright (c) 2004, 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "flock_test.h"
#include <string>
#include <fstream>
#include <functional>
#include <mt/uid.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <lfstream.h>

using namespace std;

flock_test::~flock_test()
{
  if ( !fname.empty() ) {
    unlink( fname.c_str() );
  }
}

int EXAM_IMPL(flock_test::create)
{
  fname = "/tmp/xmt-";
  fname += xmt::uid_str();

  int fd = open( fname.c_str(), O_RDWR | O_CREAT | O_APPEND, 00666 );
  EXAM_CHECK( fd >= 0 );
  
  char buf[1024];
  const int factor = 4;

  fill( buf, buf + sizeof(buf), 'a' );

  int cnt = 0;

  for ( int i = 0; i < factor; ++i ) {
    cnt += write( fd, buf, sizeof(buf) );
  }

  EXAM_CHECK( cnt == (factor * sizeof(buf)) );

  EXAM_CHECK( close( fd ) == 0 );

  return EXAM_RESULT;
}

int EXAM_IMPL(flock_test::read_lock)
{

  return EXAM_RESULT;
}

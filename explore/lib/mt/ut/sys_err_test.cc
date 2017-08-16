// -*- C++ -*-

/*
 * Copyright (c) 2008, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "sys_err_test.h"

#if defined(__GNUC__) && (__GNUC__ < 5)
#  include <mt/system_error>
#else
#  include <system_error>
#endif
#include <cerrno>
#include <string>
#include <cstring>

#include <stdio.h>

using namespace std;

int EXAM_IMPL(sys_err_test::file)
{
  FILE* f = fopen( "/tmp/no-such-file", "r" );

  if ( f == 0 ) {
    system_error se1( errno, std::system_category(), string( "Test" ) );

    EXAM_CHECK( strcmp( se1.what(), "Test: No such file or directory" ) == 0 );

    system_error se2( errno, std::system_category() );

    EXAM_CHECK( strcmp( se2.what(), "No such file or directory" ) == 0 );
  } else {
    EXAM_ERROR( "file exist, but shouldn't" );
    fclose( f );
  }
  
  return EXAM_RESULT;
}

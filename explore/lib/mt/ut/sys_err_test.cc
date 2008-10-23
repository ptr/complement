// -*- C++ -*- Time-stamp: <08/07/25 10:08:26 ptr>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "sys_err_test.h"

#include <mt/system_error>
#include <cerrno>
#include <string>

// #include <iostream>

#include <stdio.h>

using namespace std;

int EXAM_IMPL(sys_err_test::file)
{
  FILE* f = fopen( "/tmp/no-such-file", "r" );

  if ( f == 0 ) {
    system_error se1( errno, std::get_posix_category(), string( "Test" ) );

    EXAM_CHECK( strcmp( se1.what(), "Test: No such file or directory" ) == 0 );

    system_error se2( errno, std::get_posix_category() );

    EXAM_CHECK( strcmp( se2.what(), "No such file or directory" ) == 0 );
  } else {
    EXAM_ERROR( "file exist, but shouldn't" );
    fclose( f );
  }
  
  return EXAM_RESULT;
}

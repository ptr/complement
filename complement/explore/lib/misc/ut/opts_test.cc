// -*- C++ -*- Time-stamp: <08/05/01 15:16:10 ptr>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "opts_test.h"

#include <misc/opts.h>

// #include <iostream>

using namespace std;

int EXAM_IMPL(opts_test::bool_option)
{
  const char* argv[] = { "name", "-h" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add( 'h', "help", "print this help message" );

  try {
    opts.parse( argc, argv );

    EXAM_CHECK( opts.is_set( 'h' ) );
  }
  catch ( const Opts::invalid_opt& e ) {
  }
  catch ( const Opts::invalid_arg& e ) {
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::bool_option_long)
{
  const char* argv[] = { "name", "--help" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add( 'h', "help", "print this help message" );

  try {
    opts.parse( argc, argv );

    EXAM_CHECK( opts.is_set( 'h' ) );
  }
  catch ( const Opts::invalid_opt& e ) {
  }
  catch ( const Opts::invalid_arg& e ) {
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::int_option)
{

  const char* argv[] = { "name", "-p", "80" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add( 'p', "port", "listen tcp port" , true);

  try {
    opts.parse( argc, argv );

    int port = 0;

    EXAM_CHECK( opts.is_set( 'p' ) );
    EXAM_CHECK( opts.get( 'p', port ) == 80 );
  }
  catch ( const Opts::invalid_opt& e ) {
  }
  catch ( const Opts::invalid_arg& e ) {
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::int_option_long)
{
  const char* argv[] = { "name", "--port=80" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add( 'p', "port", "listen tcp port" , true );

  try {
    opts.parse( argc, argv );

    int port = 0;

    EXAM_CHECK( opts.is_set( 'p' ) );
    EXAM_CHECK( opts.get( 'p', port ) == 80 );
  }
  catch ( const Opts::invalid_opt& e ) {
  }
  catch ( const Opts::invalid_arg& e ) {
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::bad_option)
{
  const char* argv[] = { "name", "-v" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add( 'h', "help", "print this help message" );

  bool exception_happens = false;

  try {
    opts.parse( argc, argv );

    EXAM_ERROR( "exception expected" );
  }
  catch ( const Opts::invalid_opt& e ) {
    exception_happens = true;
    EXAM_CHECK( e.optname == "-v" );
  }
  catch ( const Opts::invalid_arg& e ) {
  }

  EXAM_CHECK( exception_happens );

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::bad_argument)
{
  const char* argv[] = { "name", "--port=www" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add( 'p', "port", "listen tcp port" );

  bool exception_happens = false;

  try {
    opts.parse( argc, argv );

    EXAM_ERROR( "exception expected" );
  }
  catch ( const Opts::invalid_opt& e ) {
  }
  catch ( const Opts::invalid_arg& e ) {
    exception_happens = true;
    EXAM_CHECK( e.optname == "--port" );
    EXAM_CHECK( e.argname == "www" );
  }

  EXAM_CHECK( exception_happens );

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::multiple)
{
  {
    const char* argv[] = { "name", "-vvv" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'v', "verbose", "more trace messages" );

    opts.parse( argc, argv );

    EXAM_CHECK( opts.get_cnt('v') == 3 );
  }

  {
    const char* argv[] = { "name", "-v", "-v", "-v" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'v', "verbose", "more trace messages" );

    opts.parse( argc, argv );

    EXAM_CHECK( opts.get_cnt('v') == 3 );
  }

  {
    const char* argv[] = { "name", "--verbose", "--verbose", "--verbose" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'v', "verbose", "more trace messages" );

    opts.parse( argc, argv );

    EXAM_CHECK( opts.get_cnt('v') == 3 );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::compound)
{
  {
    const char* argv[] = { "name", "-abc" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'a', "a-option", "option a" );
    opts.add( 'b', "b-option", "option b" );
    opts.add( 'c', "c-option", "option c" );

    opts.parse( argc, argv );

    EXAM_CHECK(opts.is_set('a') && opts.is_set('b') && opts.is_set('c'));
  }


  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::args)
{
  {
    const char* argv[] = { "name", "-f", "filename.conf", "file1", "file2"  };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'f', "config", "configuration file",true );

    opts.parse( argc, argv );

    EXAM_CHECK( argc == 3 );
    EXAM_CHECK( argv[0] == "name" );
    EXAM_CHECK( argv[1] == "file1" );
    EXAM_CHECK( argv[2] == "file2" );
  }

  {
    const char* argv[] = { "name", "file1", "file2", "-f", "filename.conf" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'f', "config", "configuration file",true );

    opts.parse( argc, argv );

    EXAM_CHECK( argc == 3 );
    EXAM_CHECK( argv[0] == "name" );
    EXAM_CHECK( argv[1] == "file1" );
    EXAM_CHECK( argv[2] == "file2" );
  }

  {
    const char* argv[] = { "name", "file1", "-f", "filename.conf", "file2" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'f', "config", "configuration file",true );

    opts.parse( argc, argv );

    EXAM_CHECK( argc == 3 );
    EXAM_CHECK( argv[0] == "name" );
    EXAM_CHECK( argv[1] == "file1" );
    EXAM_CHECK( argv[2] == "file2" );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::stop)
{
  {
    const char* argv[] = { "name", "-a", "--", "-f" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'a', "a-option", "option a" );

    opts.parse( argc, argv );

    EXAM_CHECK( argc == 2 );
    EXAM_CHECK( argv[1] == "-f" );
  }

  return EXAM_RESULT;
}

struct point
{
  point( int _x = 0, int _y = 0 ) :
      x(_x),
      y(_y)
      { }

    int x;
    int y;
};

istream& operator >>( istream& s, point& p )
{
  s >> p.x >> p.y;

  return s;
}
 
ostream& operator <<( ostream& s, const point& p )
{
  s << p.x << ' ' << p.y;

  return s;
}

int EXAM_IMPL(opts_test::user_defined)
{
  {
    const char* argv[] = { "name", "-s", "1 2" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 's', "start-point", "start point", true );

    opts.parse( argc, argv );

    point p( 1, 1 );

    opts.get( 's', p );

    EXAM_CHECK( (p.x == 1) && (p.y = 2) );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::reduction)
{
  {
    const char* argv[] = { "name" , "--num" , "4"};
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add('n',"number_of_processors","number of processors",true );

    opts.parse( argc, argv );

    int n;
    opts.get('n',n);
    EXAM_CHECK( n == 4 );
  }

  return EXAM_RESULT; 
}

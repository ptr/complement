// -*- C++ -*- Time-stamp: <08/05/21 12:20:14 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "opts_test.h"

#include <misc/opts.h>
#include <set>
#include <vector>

// #include <iostream>

using namespace std;

int EXAM_IMPL(opts_test::bool_option)
{
  const char* argv[] = { "name", "-h" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.addflag( 'h', "help", "print this help message" );

  try {
    opts.parse( argc, argv );

    EXAM_CHECK( opts.is_set( 'h' ) );
  }
  catch ( const Opts::unknown_option& e ) {
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

  opts.addflag( 'h', "help", "print this help message" );

  try {
    opts.parse( argc, argv );

    EXAM_CHECK( opts.is_set( 'h' ) );
  }
  catch ( const Opts::unknown_option& e ) {
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

  opts.add( 'p', 0,"port", "listen tcp port");

  try {
    opts.parse( argc, argv );

    EXAM_CHECK( opts.is_set( 'p' ) );
    EXAM_CHECK( opts.get<int>( 'p' ) == 80 );
  }
  catch ( const Opts::unknown_option& e ) {
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

  opts.add( 'p', 0, "port", "listen tcp port");

  try {
    opts.parse( argc, argv );


    EXAM_CHECK( opts.is_set( 'p' ) );
    EXAM_CHECK( opts.get<int>( 'p' ) == 80 );
  }
  catch ( const Opts::unknown_option& e ) {
  }
  catch ( const Opts::invalid_arg& e ) {
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::defaults)
{
  const char* argv[] = { "name" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add( 'p', 0, "port", "listen tcp port");

  try {
    opts.parse( argc, argv );


    EXAM_CHECK( !opts.is_set( 'p' ) );
    EXAM_CHECK( opts.get<int>( 'p' ) == 0 );
  }
  catch ( const Opts::unknown_option& e ) {
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

  opts.addflag( 'h', "help", "print this help message" );

  bool exception_happens = false;

  try {
    opts.parse( argc, argv );

    EXAM_ERROR( "exception expected" );
  }
  catch ( const Opts::unknown_option& e ) {
    exception_happens = true;
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

  opts.add( 'p', 10, "port", "listen tcp port" );

  bool exception_happens = false;
  
  opts.parse( argc, argv );

  try
  {
    int t = opts.get<int>('p');
  }
  catch(const Opts::invalid_arg& e)
  {
    exception_happens = true;
  }

  EXAM_CHECK( exception_happens );
  EXAM_CHECK ( opts.get_default<int>('p') == 10 );

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::unexpected_argument)
{
  const char* argv[] = { "name", "--help=10" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.addflag('h',"help");

  bool exception_happens = false;
  
  try
  {
    opts.parse( argc, argv );
  }
  catch(const Opts::invalid_arg& e)
  {
    exception_happens = true;
  }

  EXAM_CHECK( exception_happens );

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::missing_argument)
{
  const char* argv[] = { "name", "-n" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add('n',10,"num");

  bool exception_happens = false;
  
  try
  {
    opts.parse( argc, argv );
  }
  catch(const Opts::missing_arg& e)
  {
    exception_happens = true;
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

    opts.addflag( 'v', "verbose", "more trace messages" );

    opts.parse( argc, argv );

    EXAM_CHECK( opts.get_cnt('v') == 3 );
  }

  {
    const char* argv[] = { "name", "-v", "-v", "-v" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.addflag( 'v', "verbose", "more trace messages" );

    opts.parse( argc, argv );

    EXAM_CHECK( opts.get_cnt('v') == 3 );
  }

  {
    const char* argv[] = { "name", "--verbose", "--verbose", "--verbose" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.addflag( 'v', "verbose", "more trace messages" );

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

    opts.addflag( 'a', "a-option", "option a" );
    opts.addflag( 'b', "b-option", "option b" );
    opts.addflag( 'c', "c-option", "option c" );

    opts.parse( argc, argv );

    EXAM_CHECK(opts.is_set('a') && opts.is_set('b') && opts.is_set('c'));
  }


  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::multiple_compound)
{
  {
    const char* argv[] = { "name", "-xf","--flag", "-f", "-p=first" ,"--pa","second" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.addflag( 'x', "x-option", "option x" );
    opts.addflag( 'f', "flag", "option f" );
  
    opts.add('p',"defaultpath","path","some path");
    
    opts.parse( argc, argv );

    EXAM_CHECK(opts.is_set('x'));
    EXAM_CHECK(opts.is_set("flag"));
    EXAM_CHECK(opts.is_set('p'));
    EXAM_CHECK(opts.get_cnt("flag") == 3 && opts.get_cnt('f') == 3);
    vector<string> vs(2);

    opts.getemall("path",vs.begin());
    EXAM_CHECK( vs[0] == "first" );
    EXAM_CHECK( vs[1] == "second" );
  }


  return EXAM_RESULT;
}



int EXAM_IMPL(opts_test::args)
{
  {
    const char* argv[] = { "name", "-f", "filename.conf", "file1", "file2"  };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add( 'f',string("default.conf"), "config", "configuration file");

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

    opts.add( 'f', string("default.conf"), "config", "configuration file" );

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

    opts.add( 'f', string("default.conf"), "config", "configuration file" );

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

    opts.addflag( 'a', "a-option", "option a" );

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

    opts.add( 's', point(1,1) ,"start-point", "start point");

    opts.parse( argc, argv );

    point p = opts.get<point>( 's' );

    EXAM_CHECK( (p.x == 1) && (p.y == 2) );
  }

  return EXAM_RESULT;
}

// check whether autocomplement works
int EXAM_IMPL(opts_test::autocomplement)
{
  {
    const char* argv[] = { "name" , "--num" , "4"};
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add('n',1,"number_of_processors","number of processors" );

    opts.parse( argc, argv );

    EXAM_CHECK( opts.get<int>('n') == 4 );
  }

  return EXAM_RESULT; 
}

int EXAM_IMPL(opts_test::autocomplement_failure)
{
  {
    const char* argv[] = { "name" , "--proc" , "4"};
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add('p',1,"proc_num","process number" );
    opts.add('t',string("standart"),"proc_type","process type");

    bool exception_happens = false;
    
    try
    {
      opts.parse( argc, argv );
    }
    catch(const Opts::unknown_option& e)
    {
      exception_happens = true;
    }

    EXAM_CHECK( exception_happens );
  }

  return EXAM_RESULT; 
}

int EXAM_IMPL(opts_test::multiple_args)
{
  {
    const char* argv[] = { "name" , "-I" , "first","-I","second","-I","third"};
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.add('I',"/usr/include","include","include paths" );

    opts.parse( argc, argv );

    vector<string> vs(10);
  
    opts.getemall('I',vs.begin());
    
    EXAM_CHECK( opts.get_default<string>("include") == "/usr/include");
   
    EXAM_CHECK( vs[0] == "first" );
    EXAM_CHECK( vs[1] == "second" );
    EXAM_CHECK( vs[2] == "third" );
  }

  return EXAM_RESULT; 
}

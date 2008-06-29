// -*- C++ -*- Time-stamp: <08/06/28 10:25:10 ptr>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Copyright (c) 2008
 * Dmitry Osmakov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "opts_test.h"

#include <misc/opts.h>
#include <set>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>

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
  catch ( const Opts::arg_typemismatch& e ) {
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
  catch ( const Opts::arg_typemismatch& e ) {
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
  catch ( const Opts::arg_typemismatch& e ) {
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
  catch ( const Opts::arg_typemismatch& e ) {
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::add_check_flag)
{
  const char* argv[] = { "name", "-ht" , "--temp"};
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;
  
  int f_token = opts.addflag('f');
  opts.addflag('t',"tag");
  int foo_token = opts.addflag("foo");
  opts.addflag("temp","temp desc");
  int h_token = opts.addflag( 'h', "help", "print this help message" );
  bool exception_happens = false;

  try {
    opts.parse( argc, argv );

    EXAM_CHECK( opts.is_set( 'h' ) );
  }
  catch ( const Opts::unknown_option& e ) {
  }
  catch ( const Opts::arg_typemismatch& e ) {
  }

  EXAM_CHECK( !opts.is_set("foo") );
  EXAM_CHECK( !opts.is_set(foo_token) );
  EXAM_CHECK( opts.is_set('h') );
  EXAM_CHECK( opts.is_set(h_token) );
  EXAM_CHECK( opts.is_set("help"));
  EXAM_CHECK( opts.is_set('t'));
  EXAM_CHECK( opts.is_set("temp") );
  EXAM_CHECK( !opts.is_set("unknow option") );
  EXAM_CHECK( !opts.is_set(42) );

  try
  {
    opts.get<int>('f');
  }
  catch(const logic_error& e)
  {
    exception_happens = true;
  }

  EXAM_CHECK( exception_happens );

  exception_happens = false;
  try
  {
    opts.get_default<int>("tag");
  }
  catch(const logic_error& e)
  {
    exception_happens = true;
  }

  EXAM_CHECK( exception_happens );

  exception_happens = false;

  try
  {
    vector< string > vs;
    opts.getemall(h_token,vs.begin());
  }
  catch( const logic_error& e)
  {
    exception_happens = true;
  }

  EXAM_CHECK( exception_happens );

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::add_get_opt)
{
  const char* argv[] = { "name", "-t" , "20" , "--name=torwalds" };
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  int t_token = opts.add('t',10);
  int name_token = opts.add("name","linus");
  int port_token = opts.add('p',80,"port");
  int num_token = opts.add("num",100,"number of elements");
 

  try {
    opts.parse( argc, argv );
  }
  catch ( const Opts::unknown_option& e ) {
  }
  catch ( const Opts::arg_typemismatch& e ) {
  }

  EXAM_CHECK( opts.is_set('t') );
  EXAM_CHECK( opts.get_cnt(t_token) == 1 );
  EXAM_CHECK( opts.get<int>('t') == 20);
  //EXAM_CHECK( opts.get_default<int>(t_token) == 10 );
  EXAM_CHECK( opts.is_set(name_token) );
  EXAM_CHECK( opts.get_cnt("name") == 1 );
  EXAM_CHECK( opts.get<string>(name_token) == "torwalds");
  EXAM_CHECK( opts.get_default<string>("name") == "linus");
  EXAM_CHECK( !opts.is_set('p') );
  //EXAM_CHECK( !opts.is_set("num") && opts.get<int>(num_token) == opts.get_default<int>("num") ) ;

  return EXAM_RESULT;
}

int EXAM_IMPL(opts_test::option_position)
{
  const char* argv[] = { "name" , "--begin" , "--f1","--f2","--end","--f1","--port=10"};
  int argc = sizeof( argv ) / sizeof(argv[0]);

  Opts opts;

  opts.add( 'p', 0, "port", "listen tcp port");
  opts.addflag("begin");
  int f1_token = opts.addflag("f1");
  int f2_token = opts.addflag("f2");
  opts.addflag("end");

  try {
    opts.parse( argc, argv );
  
    EXAM_CHECK(opts.get_cnt('p') == 1);
    EXAM_CHECK(opts.get_cnt("begin") == 1);
    EXAM_CHECK(opts.get_cnt(f1_token) == 2);
    EXAM_CHECK(opts.get_cnt(f2_token) == 1);
    EXAM_CHECK(opts.get_cnt("end") == 1);
    EXAM_CHECK(opts.get_cnt("unknown") == 0);
    EXAM_CHECK(opts.get_cnt('u') == 0);
    EXAM_CHECK(opts.get_cnt(42) == 0);

    list<int> p_l(opts.get_cnt('p'));
    vector<int> b_v(opts.get_cnt("begin"));
    vector<int> f1_v(opts.get_cnt(f1_token));
    vector<int> f2_v(opts.get_cnt(f2_token));
    vector<int> e_v(opts.get_cnt("end"));
    list<int> u_l(opts.get_cnt("unknown"));
    bool exception_happens = false;

    opts.get_pos('p',p_l.begin());
    opts.get_pos("begin",b_v.begin());
    opts.get_pos("f1",f1_v.begin());
    opts.get_pos(f2_token,f2_v.begin());
    opts.get_pos("end",e_v.begin());

    try
    {
      opts.get_pos("unknown",u_l.begin());
    }
    catch(const Opts::unknown_option& e)
    {
      exception_happens = true;
    }
    
    EXAM_CHECK( exception_happens );
    EXAM_CHECK( !b_v.empty() && !e_v.empty() && e_v[0] > b_v[0]);
    EXAM_CHECK( f1_v.size() == 2 && f1_v[0] == 2 && f1_v[1] == 5);
    EXAM_CHECK( !p_l.empty() && *p_l.begin() == 6);
    EXAM_CHECK( u_l.empty() );
  }
  catch ( const Opts::unknown_option& e ) {
  }
  catch ( const Opts::arg_typemismatch& e ) {
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
  catch ( const Opts::arg_typemismatch& e ) {
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
  catch ( const Opts::arg_typemismatch& e ) {
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
  catch(const Opts::arg_typemismatch& e)
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
  catch(const Opts::arg_typemismatch& e)
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

int EXAM_IMPL(opts_test::help)
{
  {
    const char* argv[] = { "name" , "--help" };
    int argc = sizeof( argv ) / sizeof(argv[0]);

    Opts opts;

    opts.description( "what utility do" );
    opts.author( "author" );
    opts.copyright( "copyright" );

    opts.addflag('h',"help","print this help message");
    opts.addflag("flag","some program flag");
    opts.addflag('v',"version","view program version");
    opts.add('I',"/usr/include","include","include paths" );
    opts.add('p',80,"port","listen to tcp port");
    opts.add("mode","standart","program mode");
    
    opts.parse(argc,argv);

    EXAM_CHECK(opts.is_set('h'));
    EXAM_CHECK( opts.get_pname() == "name" );

    ofstream out("help.out");
    opts.help(out);
    cout << "check file help.out" << endl;
    out.close();
  }

  return EXAM_RESULT; 
}

int EXAM_IMPL(opts_test::long_string)
{
  {
    const char* argv[] = { "name" , "--string" , "long string"};
    int argc = sizeof( argv ) / sizeof(argv[0]);
    
    Opts opts;

    opts.add('s',string("default value"),"string","some string param");
  
    EXAM_CHECK( opts.get('s') == "default value");

    opts.parse(argc,argv);

    EXAM_CHECK( opts.get('s') == "long string");
  
  }

  return EXAM_RESULT;
}


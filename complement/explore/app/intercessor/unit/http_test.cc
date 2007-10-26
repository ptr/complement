// -*- C++ -*- Time-stamp: <07/03/07 16:38:24 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 */

// #include <boost/filesystem/operations.hpp>
// #include <boost/filesystem/path.hpp>
#include "http_test.h"
#include <boost/lexical_cast.hpp>

#include <string>
#include <iostream>

#include <sstream>
// #include <fstream>
#include <iterator>
#include <unistd.h>

#include "http.h"
// #include "intercessor.h"
// #include "server.h"

// #include "dummy_srv.h"
// #include <sockios/sockmgr.h>
// #include <mt/lfstream.h>

// boost::filesystem::path dir( boost::filesystem::initial_path() );
// unsigned rq_timeout = 3;

using namespace std;
using namespace http;

int EXAM_IMPL(http_test::header_io)
{
  header h;
  string rq = "Content-Length: 100\r\n";
  stringstream s( rq );

  s >> h;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( h.key() == "Content-Length" );
  EXAM_CHECK( h.value() == "100" );

  stringstream o;

  o << h;

  EXAM_CHECK( !o.fail() );
  EXAM_CHECK( o.str() == "Content-Length: 100\r\n" );

  return EXAM_RESULT;
}

int EXAM_IMPL(http_test::header_sp)
{
  header h;
  string rq = "Header: fields here \r\n";
  stringstream s( rq );

  s >> h;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( h.key() == "Header" );
  EXAM_CHECK( h.value() == "fields here" );

  stringstream o;

  o << h;

  EXAM_CHECK( !o.fail() );
  EXAM_CHECK( o.str() == "Header: fields here\r\n" );

  return EXAM_RESULT;
}

int EXAM_IMPL(http_test::command)
{
  string cmd = "GET http://myhost.com HTTP/1.1\r\n";
  stringstream s( cmd );
  http::command c;

  s >> c;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( c.value() == command::GET );
  EXAM_CHECK( c.URL() == "http://myhost.com" );
  EXAM_CHECK( c.protocol() == command::HTTP11 );

  stringstream o;

  o << c;

  EXAM_CHECK( !o.fail() );
  EXAM_CHECK( o.str() == "GET http://myhost.com HTTP/1.1\r\n" );

  return EXAM_RESULT;
}

int EXAM_IMPL(http_test::base_response)
{
  string rs = "HTTP/1.1 202 Accepted\r\n";
  stringstream s( rs );
  http::base_response r;

  s >> r;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( r.code() == 202 );
  EXAM_CHECK( r.protocol() == http::command::HTTP11 );

  stringstream o;

  o << r;

  EXAM_CHECK( !o.fail() );
  EXAM_CHECK( o.str() == "HTTP/1.1 202 Accepted\r\n" );

  return EXAM_RESULT;
}

int EXAM_IMPL(http_test::request)
{
  string rq = "GET /index.html HTTP/1.1\r\n"
              "Host: myhost.com\r\n"
              "X-Header: test\r\n"
              "\r\n";
  stringstream s( rq );
  http::request r;

  s >> r;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( r.head().protocol() == http::command::HTTP11 );
  EXAM_CHECK( r.head().value() == http::command::GET );
  EXAM_CHECK( r.head().URL() == "/index.html" );
  EXAM_CHECK( r.headers().size() == 2 );
  EXAM_CHECK( r.headers().begin()->key() == "Host" );
  EXAM_CHECK( r.headers().begin()->value() == "myhost.com" );

  EXAM_CHECK( r.search( "Host" ) == r.headers().begin() );

  return EXAM_RESULT;
}

int EXAM_IMPL(http_test::response)
{
  string rq = "HTTP/1.1 202 Accepted\r\n"
              "Content-Length: 100\r\n"
              "\r\n";
  stringstream s( rq );
  http::response r;

  s >> r;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( r.head().protocol() == http::command::HTTP11 );
  EXAM_CHECK( r.head().code() == 202 );
  EXAM_CHECK( r.headers().size() == 1 );
  EXAM_CHECK( r.headers().begin()->key() == "Content-Length" );
  EXAM_CHECK( boost::lexical_cast<int>( r.headers().begin()->value() ) == 100 );

  EXAM_CHECK( r.search( "Content-Length" ) == r.headers().begin() );

  return EXAM_RESULT;
}


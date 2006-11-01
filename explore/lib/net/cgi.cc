// -*- C++ -*- Time-stamp: <02/09/29 20:22:07 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$Id$"
#  else
#pragma ident "$Id$"
#  endif
#endif

#include <config/feature.h>

#include "net/cgi.h"
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

namespace cgi {

const char *SERVER_SOFWARE    = "SERVER_SOFWARE";
const char *SERVER_NAME       = "SERVER_NAME";
const char *GATEWAY_INTERFACE = "GATEWAY_INTERFACE";
const char *SERVER_PROTOCOL   = "SERVER_PROTOCOL";
const char *SERVER_PORT       = "SERVER_PORT";
const char *PATH_INFO         = "PATH_INFO";
const char *PATH_TRANSLATED   = "PATH_TRANSLATED";
const char *SCRIPT_NAME       = "SCRIPT_NAME";
const char *REQUEST_METHOD    = "REQUEST_METHOD";
const char *QUERY_STRING      = "QUERY_STRING";
const char *REMOTE_HOST       = "REMOTE_HOST";
const char *REMOTE_ADDR       = "REMOTE_ADDR";
const char *AUTH_TYPE         = "AUTH_TYPE";
const char *REMOTE_USER       = "REMOTE_USER";
const char *REMOTE_IDENT      = "REMOTE_IDENT";
const char *CONTENT_TYPE      = "CONTENT_TYPE";
const char *CONTENT_LENGTH    = "CONTENT_LENGTH";
const char *HTTP_USER_AGENT   = "HTTP_USER_AGENT";

const char *EMPTY = "";

const string EMPTY_STR;

environment::environment() :
    server_port( 0 ),
    content_length( 0 )
{
  server_software = get( SERVER_SOFWARE );
  server_name = get( SERVER_NAME );
  gateway_interface = get( GATEWAY_INTERFACE );

  server_protocol = get( SERVER_PROTOCOL );
  {
    istringstream ss( get( SERVER_PORT ) );
    ss >> server_port;
  }
  // server_port = get( SERVER_PORT );
  path_info = get( PATH_INFO );
  path_translated = get( PATH_TRANSLATED );
  script_name = get( SCRIPT_NAME );
  request_method = get( REQUEST_METHOD );
  query_string = get( QUERY_STRING );
  query_string = decode( query_string );


  remote_host = get( REMOTE_HOST );
  remote_addr = get( REMOTE_ADDR );
  auth_type = get( AUTH_TYPE );
  remote_user = get( REMOTE_USER );
  remote_ident = get( REMOTE_IDENT );
  content_type = get( CONTENT_TYPE );
  {
    stringstream ss( get( CONTENT_LENGTH ) );

    ss >> content_length;
  }

  // Check that this is valid POST request, and parse it
  if ( request_method == "POST" && content_length > 0 &&
    content_type == "application/x-www-form-urlencoded" ) {
    stringstream tmp;
    tmp << cin.rdbuf();
    string var;
    string val;

    while ( tmp.good() ) {
      getline( tmp, var, '=' );
      getline( tmp, val, '&' );
      pars.push_back( value_type(decode(var),decode(val)) );
      var.clear();
      val.clear();
    }
  } else if ( request_method == "GET" ) {
  }

  http_user_agent = get( HTTP_USER_AGENT );
}

const char *environment::get( const char *name )
{
  char *tmp = getenv( name );
  return tmp == 0 ? EMPTY : tmp;
}

string environment::decode( const string& query )
{
  string::size_type p = 0;
  string::size_type pb = 0;
  unsigned tmp;
  string s;
  stringstream ss;
  ss.setf( ios::hex, ios::basefield );
  while ( (p = query.find_first_of( "%+", p )) != string::npos ) {
    s.append( query, pb, p - pb );
    if ( query.at( p ) == '%' ) {
      ss.str( query.substr( p + 1, 2 ) );
      ss.clear();
      ss >> tmp;
      s.append( 1, (char)tmp );

      p += 3;
    } else {
      s.append( 1, ' ' );
      ++p;
    }
    pb = p;
  }
  s.append( query, pb, query.length() - pb );

  return s;
}

string environment::hexify( const string& s )
{
  stringstream ss;
  ss << hex;

  const char *ch = s.data();
  const char *end = s.data() + s.length();
  while ( ch != end ) {
    ss << setfill( '0' ) << setw( 2 )
       << ( (unsigned)*(const unsigned char *)ch++ );
  }

  return ss.str();
}

string environment::unhexify( const string& s )
{
  stringstream ss;
  ss << hex;
  unsigned tmp;
  string out;
  string::size_type p = 0;

  while ( p < s.length() && !ss.fail() ) {
    ss.str( s.substr( p, 2 ) );
    ss.clear();
    ss >> tmp;
    out.append( 1, (char)tmp );
    p += 2;
  }

  return out;
}

const string& environment::value( const char *var ) const
{
  container_type::const_iterator i = pars.begin();

  while ( i != pars.end() ) {
    if ( (*i).first == var ) {
      return (*i).second;
    }
    ++i;
  }

  return EMPTY_STR;
}

} // namespace cgi

// -*- C++ -*- Time-stamp: <05/09/11 11:43:36 ptr>

/*
 * Copyright (c) 2002, 2005
 * Petr Ovtchenkov
 *
 * Copyright (c) 2001
 * ParallelGraphics Ltd.
 * 
 * Licensed under the Academic Free License Version 2.1
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

#include <config/feature.h>
#include "DB/xxSQL_i.h"

#include <cstdio>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace xxSQL {

using std::string;
using std::vector;
using namespace std;

DataBase_connect::DataBase_connect() :
    name(),
    user(),
    pass(),
    host(),
    opt(),
    tty(),
    err( 0 ),
    port( 0 )
{  
}

DataBase::DataBase( const DataBase_connect& c ) :
    _flags( 0 ),
    _dberr( c.err != 0 ? *c.err : std::cerr ),
    _xc( 0 )
{
  if ( c.name.length() != 0 ) {
    _dbname = c.name;
  }
  if ( c.user.length() != 0 ) {
    _dbusr = c.user;
  }
  if ( c.pass.length() != 0 ) {
    _dbpass = c.pass;
  }
  if ( c.host.length() != 0 ) {
    _dbhost = c.host;
  }
  if ( c.port != 0 ) {
    // 
  } else {
    _dbport = ~0U;
  }
  if ( c.opt.length() != 0 ) {
    _dbopt = c.opt;
  }
  if ( c.tty.length() != 0 ) {
    _dbtty = c.tty;
  }
}

DataBase::~DataBase()
{
}

Cursor::Cursor( const char *nm ) :
    name( nm )
{
}

Cursor::~Cursor()
{
}

string lower( const string& s )
{
  string l;
  l.reserve( s.length() );
  for ( string::const_iterator i = s.begin(); i != s.end(); ++i ) {
    l += tolower( *i );
  }
  return l;
}

string lower( const char *s )
{
  string l;
  unsigned n = strlen(s);
  const char *e = s + n;
  l.reserve( n );
  while ( s != e ) {
    l += tolower( *s++ );
  }
  return l;
}

const string& Cursor::value( int n, const string& f )
{
  vector<string>::iterator i = find( fields.begin(), fields.end(), lower(f) );
  if ( i == fields.end() ) {
    throw std::invalid_argument( f );
  }
  vector<string>::size_type j = 0;
  distance( fields.begin(), i, j );
  return data[n][j];
}

const string& Cursor::value( int n, const char *cf )
{
  // string f( cf );
  vector<string>::iterator i = find( fields.begin(), fields.end(), lower(cf) );
  if ( i == fields.end() ) {
    throw std::invalid_argument( cf );
  }
  vector<string>::size_type j = 0;
  distance( fields.begin(), i, j );
  return data[n][j];
}

} // namespace xxSQL

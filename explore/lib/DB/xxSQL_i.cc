// -*- C++ -*- Time-stamp: <01/03/19 19:17:20 ptr>

/*
 *
 * Copyright (c) 2001
 * ParallelGraphics
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

#include <config/feature.h>
#include "DB/xxSQL_i.h"

#include <cstdio>

#include <iostream>
#include <sstream>
#include <iomanip>

namespace xxSQL {

using std::string;
using std::vector;
using namespace std;

DataBase::DataBase( const char *name, const char *usr, const char *passwd,
                    const char *host, const char *port,
                    const char *opt, const char *tty,
                    std::ostream *err ) :
    _flags( 0 ),
    _dberr( err != 0 ? *err : std::cerr )
{
  if ( name != 0 ) {
    _dbname = name;
  }
  if ( usr != 0 ) {
    _dbusr = usr;
  }
  if ( passwd != 0 ) {
    _dbpass = passwd;
  }
  if ( host != 0 ) {
    _dbhost = host;
  }
  if ( port != 0 ) {
    // 
  } else {
    _dbport = ~0U;
  }
  if ( opt != 0 ) {
    _dbopt = opt;
  }
  if ( tty != 0 ) {
    _dbtty = tty;
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

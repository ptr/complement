// -*- C++ -*- Time-stamp: <00/10/13 10:17:07 ptr>

/*
 *
 * Copyright (c) 2000
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
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#include <config/feature.h>
#include "DB/xxSQL_i.h"

#include <cstdio>

#include <iostream>
#include <sstream>
#include <iomanip>

namespace xxSQL {

using __STD::string;
using __STD::vector;
using namespace __STD;

DataBase::DataBase( const char *name, const char *usr, const char *passwd,
                    const char *host, const char *port,
                    const char *opt, const char *tty ) :
    _flags( 0 )
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

const string& Cursor::value( int n, const string& f )
{
  vector<string>::iterator i = find( fields.begin(), fields.end(), f );
  if ( i == fields.end() ) {
    throw __STD::invalid_argument( f );
  }
  vector<string>::size_type j = 0;
  distance( fields.begin(), i, j );
  return data[n][j];
}

const string& Cursor::value( int n, const char *cf )
{
  string f( cf );
  vector<string>::iterator i = find( fields.begin(), fields.end(), f );
  if ( i == fields.end() ) {
    throw __STD::invalid_argument( f );
  }
  vector<string>::size_type j = 0;
  distance( fields.begin(), i, j );
  return data[n][j];
}

} // namespace xxSQL

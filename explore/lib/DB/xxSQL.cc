// -*- C++ -*- Time-stamp: <05/10/03 22:52:16 ptr>

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
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

#include "DB/xxSQL.h"

#include <sstream>
#include <iomanip>
#include <dlfcn.h>

namespace xxSQL {

// extern "C" void *(*DBimpl)( const void * );
typedef void *(*DBimpl_type)( const void * );

#ifdef _STLP_DEBUG
const char *lname_pg  = "libDBpgstlg.so";
const char *lname_ora = "libDBorastlg.so";
#elif defined(__DEBUG)
const char *lname_pg  = "libDBpgg.so";
const char *lname_ora = "libDBorag.so";
#else
const char *lname_pg  = "libDBpg.so";
const char *lname_ora = "libDBora.so";
#endif

DBxx::DBxx( DBvendor vendor, const DataBase_connect& conn ) :
    lh( 0 ),
    _dbvendor( Unknown ),
    fmtflags( 0 )
{
  try {
    DBimpl_type DBimpl;
    /*
      DB-specific libraries are loaded dynamically at runtime now,
      via dlopen, etc. So we can detect what DB we use only at runtime. 
     */

    /*
      Unfortunately, we still depend from compiler: C++ name mangling
      scheme are differ for different compilers, and ones can even depends from
      compilation options. So for DB wrapper library we SHOULD use
      the same compiler (in common) as for this interface library.
      This lead to problems with building third-party DB wrapper
      libraries, but this is common C++ problem.
     */
    switch ( vendor ) {
      case PostgreSQL:
        lh = dlopen( lname_pg, RTLD_LAZY );
        if ( lh == 0 ) {
          throw vendor;
        }
        break;
      case Oracle8i:
        lh = dlopen( lname_ora, RTLD_LAZY );
        if ( lh == 0 ) {
          throw vendor;
        }
        break;
      default:
        _db = 0;
        throw Unknown;
    }
    DBimpl = (DBimpl_type)dlsym( lh, "DataBase" );
    _db = reinterpret_cast<DataBase *>( (*DBimpl)( reinterpret_cast<const void *>(&conn) ) );
    _dbvendor = vendor;
  }
  catch ( const DBvendor& v ) {
    switch ( v ) {
      case PostgreSQL:
        *conn.err << "Can't find " << lname_pg << endl;
        break;
      case Oracle8i:
        *conn.err << "Can't find " << lname_ora << endl;
        break;
      default:
        *conn.err << "Unsupported database" << endl;
        break;
    }
  }
}

DBxx::~DBxx()
{
  delete _db;
  if ( lh != 0 ) {
    dlclose( lh );
    lh = 0;
  }
}

string DBxx::escape_data( const string& s ) const
{
  string r( s );

  // escape single quote
  string::size_type p = r.find( '\'', 0 );
  while ( p != string::npos ) {
    r.insert( p, 1, '\'' );
    p += 2;
    p = r.find( '\'', p );
  }

  return r;
}

string DBxx::escape_data( const char *s ) const
{
  string r( s );

  // escape single quote
  string::size_type p = r.find( '\'', 0 );
  while ( p != string::npos ) {
    r.insert( p, 1, '\'' );
    p += 2;
    p = r.find( '\'', p );
  }

  return r;
}

string DBxx::get_time() const
{
  if ( _db != 0 ) {
    return _db->get_time();
  }
  return get_time( time(0) );
}

string DBxx::get_time( time_t t ) const
{
  if ( _db != 0 ) {
    return _db->get_time();
  }

  std::ostringstream os;
  tm tstr;
  localtime_r( &t, &tstr );
  os << (tstr.tm_year + 1900)
     << '-' << setw( 2 ) << setfill( '0' ) << (tstr.tm_mon+1)
     << '-' << setw( 2 ) << setfill( '0' ) << tstr.tm_mday
     << ' ' << setw( 2 ) << setfill( '0' ) << tstr.tm_hour
     << ':' << setw( 2 ) << setfill( '0' ) << tstr.tm_min
     << ':' << setw( 2 ) << setfill( '0' ) << tstr.tm_sec;
  return os.str();
}

string DBxx::IS_NULL() const
{
  if ( _db != 0 ) {
    return _db->IS_NULL();
  }
  return string( "IS NULL" );
}

string DBxx::IS_NOT_NULL() const
{
  if ( _db != 0 ) {
    return _db->IS_NOT_NULL();
  }
  return string( "IS NOT NULL" );
}

} // namespace xxSQL

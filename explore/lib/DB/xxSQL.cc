// -*- C++ -*- Time-stamp: <01/02/13 12:23:58 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#include <config/feature.h>

#include "DB/xxSQL.h"
#ifdef __DB_POSTGRES
#  include "DB/PgSQL.h"
#endif
#ifdef __DB_ORACLE
#  include "DB/OraSQL.h"
#endif

#include <sstream>
#include <iomanip>

namespace xxSQL {

DBxx::DBxx( DBvendor vendor,
            const char *name, const char *usr, const char *passwd,
            const char *host, const char *port,
            std::ostream *err,
            const char *opt,
            const char *tty )
{
  switch ( vendor ) {
    case PostgreSQL:
#ifdef __DB_POSTGRES
      _db = new PgSQL::DataBase(name,usr,passwd,host,port,opt,tty,err);
#else
      _db = 0;
#endif
      break;
    case Oracle8i:
#ifdef __DB_ORACLE
      _db = new OraSQL::DataBase(name,usr,passwd,err);
#else
      _db = 0;
#endif
      break;
    default:
      _db = 0;
      break;
  }
}

DBxx::~DBxx()
{
  delete _db;
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

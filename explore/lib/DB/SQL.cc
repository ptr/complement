// -*- C++ -*- Time-stamp: <00/02/21 16:57:08 ptr>

#ident "$SunId$ %Q%"

#include <config/feature.h>
#include "DB/SQL.h"

#if defined( __DB_POSTGRES )
#  include <cstdio>
using std::FILE;
#  include <libpq-fe.h>
#elif defined( __DB_MYSQL )
#  include <mysql.h>
#endif

#include <iostream>
#include <iomanip>

namespace database {

using __STD::string;
using __STD::vector;
using namespace __STD;

NILL_type NILL;

void Table::done()
{
  __STD::ostringstream s;
  bool is_first = true;
  switch ( _op ) {
    case _INSERT:
#if defined( __DB_POSTGRES )
      s << "INSERT INTO \"" << _name << "\" (";
#elif defined( __DB_MYSQL )
      s << "INSERT INTO " << _name << " (";
#endif
      for ( int i = 0; i < header.size(); ++i ) {
        if ( header[i].second == _NILL ) {
          continue;
        }
        if ( !is_first ) {
          s << ",";
        } else {
          is_first = false;
        }
#if defined( __DB_POSTGRES )
        s << "\"" << header[i].first << "\"";
#elif defined( __DB_MYSQL )
        s << header[i].first;
#endif
      }
      is_first = true;
      s << ") VALUES (";
      for ( int i = 0; i < row.size(); ++i ) {
        if ( header[i].second == _NILL ) {
          continue;
        }
        if ( !is_first ) {
          s << ",";
        } else {
          is_first = false;
        }
        s << row[i];
      }
      s << ")";
      db->state( s.str() );
      break;
    default:
      throw invalid_argument( "database::Table no op" );
      break;
  }
}

DataBase::DataBase( const char *name, const char *usr, const char *passwd,
                    const char *host, const char *port,
                    const char *opt, const char *tty ) :
    _flags( 0 )
{
#if defined( __DB_POSTGRES )
  string db_name_quoted = "\"";
  db_name_quoted += name;
  db_name_quoted += "\"";
  _conn = PQsetdbLogin( host, port, opt, tty, /* name */ db_name_quoted.c_str(), usr, passwd );
  if ( PQstatus( _conn ) == CONNECTION_BAD ) {
    _flags = badbit | failbit;
    PQfinish( _conn );
    _conn = 0;
  }
#elif defined( __DB_MYSQL )
  _conn = new DBconn;
  mysql_init( _conn );
  int _port = 0;
  if ( port != 0 ) {
    istringstream is( port );
    is >> _port;
  }
  if ( !mysql_real_connect( _conn, host, usr, passwd, name, _port, 0, 0 ) ) {
    _flags = badbit | failbit;
    mysql_close( _conn );
    delete _conn;
    _conn = 0;
  }
#endif
}

DataBase::~DataBase()
{
  if ( _flags == goodbit ) {
#if defined( __DB_POSTGRES )
    PQfinish( _conn );
#elif defined( __DB_MYSQL )
    mysql_close( _conn );
    delete _conn;
#endif
    _conn = 0;
    _flags = badbit;
  }
}

void DataBase::exec( const string& s )
{
  if ( _flags != goodbit ) {
    return;
  }

#if defined( __DB_POSTGRES )
  PGresult *_result = PQexec( _conn, s.c_str() );
  if ( _result == 0 || PQresultStatus( _result ) != PGRES_COMMAND_OK ) {
    _flags = failbit;
  }

  PQclear( _result );
#elif defined( __DB_MYSQL )
  if ( mysql_query( _conn, s.c_str() ) ) {
    _flags = failbit;
  }
#endif
}

void DataBase::exec()
{
  if ( _flags != goodbit ) {
    return;
  }

#if defined( __DB_POSTGRES )
  PGresult *_result = PQexec( _conn, sentence.c_str() );
  if ( _result == 0 || PQresultStatus( _result ) != PGRES_COMMAND_OK ) {
    _flags = failbit;
  }

  PQclear( _result );
#elif defined( __DB_MYSQL )
  if ( mysql_query( _conn, sentence.c_str() ) ) {
    _flags = failbit;
  }
#endif
}

Table& DataBase::define_table( const char *nm )
{
  tables.push_back( new Table( nm, this ) );

  return *tables.back(); // **(--tables.end());
}

Table& DataBase::table( const char *nm )
{
  tbl_container_type::iterator i = tables.begin();
  while ( i != tables.end() ) {
    if ( (*i)->name() == nm ) {
      return **i;
    }
    ++i;
  }
  string err( "No table " );
  err += nm;
  err += " defined";
  throw logic_error( err );
}

void DataBase::begin_transaction()
{
#if defined( __DB_POSTGRES )
  sentence = "BEGIN";
#elif defined( __DB_MYSQL )
  sentence.clear(); // LOCK?
#endif
}

void DataBase::end_transaction()
{
#if defined( __DB_POSTGRES )
  sentence += "; COMMIT";
#elif defined( __DB_MYSQL )

#endif
  this->exec();
}

Transaction::Transaction( DataBase& db ) :
    _db( db )
{
  if ( _db.good() ) {
#if defined( __DB_POSTGRES )
    _exec_str = "BEGIN";
    _db.exec( _exec_str );
#elif defined( __DB_MYSQL )

#endif
  }
}

Transaction::~Transaction()
{
  if ( _db.good() ) {
#if defined( __DB_POSTGRES )
    _exec_str = "COMMIT";
    _db.exec( _exec_str );
#elif defined( __DB_MYSQL )

#endif
  }
}

Cursor::Cursor( Transaction& t , const char *nm, const char *select ) :
    name( nm ),
    tr( t )
{
#if defined( __DB_POSTGRES )
  string tmp = "DECLARE ";
  tmp += name;
  tmp += " CURSOR FOR ";
  tmp += select;
  tr._db.exec( tmp );
  cerr << tmp << endl;
#elif defined( __DB_MYSQL )
  name = select;
#endif
}

Cursor::~Cursor()
{
#if defined( __DB_POSTGRES )
  string tmp = "CLOSE ";
  tmp += name;
  tr._db.exec( tmp );
#elif defined( __DB_MYSQL )

#endif
}

void Cursor::fetch_all()
{
#if defined( __DB_POSTGRES )
  string tmp = "FETCH ALL IN ";
  tmp += name;
  PGresult *_result = PQexec( tr._db._conn, tmp.c_str() );
  if ( _result == 0 || PQresultStatus( _result ) != PGRES_TUPLES_OK ) {
    PQclear( _result );
    _result = 0;
    tr._db._flags = DataBase::failbit;
  }
  fields.clear();
  int n = PQnfields( _result );
  fields.reserve( n );

  for ( int i = 0; i < n; ++i ) {
    fields.push_back( PQfname( _result, i ) );
  }
  int ntuples = PQntuples( _result );
  data.clear();
  data.reserve( ntuples );
  for ( int j = 0; j < ntuples; ++j ) {
    data.push_back();
    data[j].reserve( n );
    for ( int i = 0; i < n; ++i ) {
      data[j].push_back( PQgetvalue( _result, j, i ) );
    }
  }
  PQclear( _result );
#elif defined( __DB_MYSQL )
  tr._db.exec( name );
  cerr << name << endl;
  MYSQL_RES *_result = mysql_store_result( tr._db._conn );

  if ( _result == 0 ) {
    if ( mysql_errno( tr._db._conn ) ) {
      tr._db._flags = DataBase::failbit;
    } else if ( mysql_field_count( tr._db._conn ) == 0 ) {
      // not select
      ;
    }
    fields.clear();
    return;
  }

  fields.clear();
  int n = mysql_num_fields( _result );
  fields.reserve( n );

  for ( int i = 0; i < n; ++i ) {
    fields.push_back( mysql_fetch_field( _result )->name );
  }
  int ntuples = mysql_num_rows( _result );

  data.clear();
  data.reserve( ntuples );

  MYSQL_ROW row;
  for ( int j = 0; j < ntuples; ++j ) {
    data.push_back();
    data[j].reserve( n );
    row = mysql_fetch_row( _result );
    for ( int i = 0; i < n; ++i ) {
      data[j].push_back( row[i] );
    }
  }
  mysql_free_result( _result );
#endif
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

} // namespace database

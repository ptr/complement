// -*- C++ -*- Time-stamp: <99/09/22 10:52:56 ptr>

#ident "$SunId$ %Q%"

#include <config/feature.h>
#include "DB/SQL.h"
#include <cstdio>
using std::FILE;
#include <libpq-fe.h>

#include <iostream>
#include <iomanip>

namespace database {

using std::string;
using std::vector;
using namespace std;

DataBase::DataBase( const char *name,
                    const char *host, const char *port,
                    const char *opt, const char *tty ) :
    _flags( 0 )
{  
  _conn = PQsetdbLogin( host, port, opt, tty, name, 0, 0 );
  if ( PQstatus( _conn ) == CONNECTION_BAD ) {
    _flags = badbit | failbit;
    PQfinish( _conn );
    _conn = 0;
  }
}

DataBase::~DataBase()
{
  if ( _flags == goodbit ) {
    PQfinish( _conn );
    _conn = 0;
    _flags = badbit;
  }
}

void DataBase::exec( const string& s )
{
  if ( _flags != goodbit ) {
    return;
  }

  PGresult *_result = PQexec( _conn, s.c_str() );
  if ( _result == 0 || PQresultStatus( _result ) != PGRES_COMMAND_OK ) {
    _flags = failbit;
  }

  PQclear( _result );
}

Transaction::Transaction( DataBase& db ) :
    _db( db )
{
  if ( _db.good() ) {
    _exec_str = "BEGIN";
    _db.exec( _exec_str );
  }
}

Transaction::~Transaction()
{
  if ( _db.good() ) {
    _exec_str = "COMMIT";
    _db.exec( _exec_str );
  }
}

Cursor::Cursor( Transaction& t , const char *nm, const char *select ) :
    name( nm ),
    tr( t )
{
  string tmp = "DECLARE ";
  tmp += name;
  tmp += " CURSOR FOR ";
  tmp += select;
  tr._db.exec( tmp );
  cerr << tmp << endl;
}

Cursor::~Cursor()
{
  string tmp = "CLOSE ";
  tmp += name;
  tr._db.exec( tmp );
}

void Cursor::fetch_all()
{
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
}

const string& Cursor::value( int n, const string& f )
{
  vector<string>::iterator i = find( fields.begin(), fields.end(), f );
  if ( i == fields.end() ) {
    throw std::invalid_argument( f );
  }
  vector<string>::size_type j = 0;
  distance( fields.begin(), i, j );
  return data[n][j];
}

} // namespace database

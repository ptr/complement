// -*- C++ -*- Time-stamp: <05/09/11 12:01:55 ptr>

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

#include <config/feature.h>
#include "DB/PgSQL.h"

#include <cstdio>
using std::FILE;
#include <libpq-fe.h>

#include <iostream>
#include <sstream>
#include <iomanip>

extern "C" void *DataBase( const void *conn )
{
  return reinterpret_cast<void *>(
    new PgSQL::DataBase(
      *reinterpret_cast<const xxSQL::DataBase_connect *>(conn) ) );
}

namespace PgSQL {

using std::string;
using std::vector;
using namespace std;

DataBase::DataBase( const xxSQL::DataBase_connect& c ) :
    xxSQL::DataBase( c ),
    thr( xmt::Thread::detached )
{
  /*
    For PostgreSQL 6.5.1 (at least) we need quotes around db name,
    (libpg.so.2.0) while for 7.0 not (libpg.so.2.1).
    Something between this versions...
   */
  /*
  string db_name_quoted; = "\"";
  db_name_quoted += name;
  db_name_quoted += "\"";
  */

  con_cond.set( false );
  thr.launch( conn_proc, this );
  timespec tm;
  xmt::Thread::gettime( &tm );

  tm.tv_sec += 10; // wait 10 secs

  if ( con_cond.wait_time( &tm ) != 0 ) {
    _flags = badbit | failbit;
    _dberr << "Connect to DB timeout" << endl;
  }
}

DataBase::~DataBase()
{
  if ( _flags == goodbit ) {
    PQfinish( _conn );
    _conn = 0;
    _flags = badbit;
  }

  // thr.join(); // no join, it detached
  //if ( thr.good() ) {
  //  thr.kill( SIGTERM );
  //}
}

xmt::Thread::ret_code DataBase::conn_proc( void *p )
{
  DataBase *me = reinterpret_cast<DataBase *>(p);
  xmt::Thread::ret_code ret;
  ret.iword = 0;
  me->_conn = PQsetdbLogin( me->_dbhost.length() > 0 ? me->_dbhost.c_str() : 0,
                            0 /* _dbport */,
                            me->_dbopt.length() > 0 ? me->_dbopt.c_str() : 0,
                            me->_dbtty.length() > 0 ? me->_dbtty.c_str() : 0,
                            me->_dbname.length() > 0 ? me->_dbname.c_str() : 0,
                            me->_dbusr.length() > 0 ? me->_dbusr.c_str() : 0,
                            me->_dbpass.length() > 0 ? me->_dbpass.c_str() : 0 );
  if ( me->_conn == 0 ) {
    me->_flags = badbit | failbit;
  } else if ( PQstatus( me->_conn ) == CONNECTION_BAD ) {
    me->_flags = badbit | failbit;
    PQfinish( me->_conn );
    me->_conn = 0;
  } else {
    me->_flags = goodbit;
  }
  me->con_cond.set( true );

  return ret;
}

void DataBase::reconnect()
{
  if ( _conn != 0 ) {
    PQfinish( _conn );
  }
  con_cond.set( false );

  if ( thr.good() ) {
    timespec tm;
    xmt::Thread::gettime( &tm );

    tm.tv_sec += 2; // thread already running, so wait only 2 secs

    if ( con_cond.wait_time( &tm ) != 0 ) {
      _flags = badbit | failbit;
      _dberr << "Connect to DB timeout" << endl;
    }
  } else {
    thr.launch( (__impl::Thread::entrance_type)conn_proc, (void *)this );
    timespec tm;
    xmt::Thread::gettime( &tm );

    tm.tv_sec += 5; // wait 5 secs

    if ( con_cond.wait_time( &tm ) != 0 ) {
      _flags = badbit | failbit;
      _dberr << "Connect to DB timeout" << endl;
   }
  }
}

void DataBase::exec( const string& s )
{
  if ( !good() ) {
    return;
  }

  PGresult *_result = PQexec( _conn, s.c_str() );
  if ( _result == 0 || PQresultStatus( _result ) != PGRES_COMMAND_OK ) {
    _flags = failbit;
  }

  PQclear( _result );
}

void DataBase::exec( const char *s )
{
  if ( !good() ) {
    return;
  }

  PGresult *_result = PQexec( _conn, s );
  if ( _result == 0 || PQresultStatus( _result ) != PGRES_COMMAND_OK ) {
    _flags = failbit;
  }

  PQclear( _result );
}

void DataBase::begin_transaction()
{
  exec( "BEGIN" );
}

void DataBase::end_transaction()
{
  exec( "COMMIT" );
}

xxSQL::Cursor *DataBase::create_cursor( const char *str )
{
  ostringstream s;

  s << "CU" << hex << _xc++;

  return new Cursor( s.str().c_str(), this, str );
}

const string Pg_time_now( "'now'" );
const string Pg_IS_NULL( "ISNULL" );
const string Pg_IS_NOTNULL( "NOTNULL" );

// return quoted string
string DataBase::get_time() const
{
  return Pg_time_now;
}

// return quoted string
string DataBase::get_time( time_t t ) const
{
  // another solution is 'timestamp(t)'
  ostringstream os;
  tm tstr;
  localtime_r( &t, &tstr );
  os << '\'' << (tstr.tm_year + 1900)
     << '-' << setw( 2 ) << setfill( '0' ) << (tstr.tm_mon+1)
     << '-' << setw( 2 ) << setfill( '0' ) << tstr.tm_mday
     << ' ' << setw( 2 ) << setfill( '0' ) << tstr.tm_hour
     << ':' << setw( 2 ) << setfill( '0' ) << tstr.tm_min
     << ':' << setw( 2 ) << setfill( '0' ) << tstr.tm_sec
     << '\'';
  return os.str();
}

string DataBase::IS_NULL() const
{
  return Pg_IS_NULL;
}

string DataBase::IS_NOT_NULL() const
{
  return Pg_IS_NOTNULL;
}

Cursor::Cursor( const char *nm, DataBase *_db, const char *s ) :
    xxSQL::Cursor( nm ),
    db( _db )
{
  string tmp = "DECLARE ";
  tmp += nm;
  tmp += " CURSOR FOR ";
  tmp += s;

  db->exec( tmp );
}

Cursor::~Cursor()
{
  string tmp = "CLOSE ";
  tmp += name;
  db->exec( tmp );
}

void Cursor::fetch_all()
{
  string tmp = "FETCH ALL IN ";
  tmp += name;
  PGresult *_result = PQexec( db->_conn, tmp.c_str() );
  if ( _result == 0 || PQresultStatus( _result ) != PGRES_TUPLES_OK ) {
    PQclear( _result );
    _result = 0;
    db->_flags = DataBase::failbit;
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
    data.push_back( datarow_type() );
    data[j].reserve( n );
    for ( int i = 0; i < n; ++i ) {
      data[j].push_back( PQgetvalue( _result, j, i ) );
    }
  }
  PQclear( _result );
}

} // namespace PgSQL

// -*- C++ -*- Time-stamp: <06/05/30 00:34:59 ptr>

/*
 *
 * Copyright (c) 2006
 * Petr Ovtchenkov
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
#include "DB/MySQL.h"

#include <cstdio>
// using std::FILE;
#include <mysql/mysql.h>
#include <mysql/errmsg.h>

#ifdef IS_NOT_NULL
# undef IS_NOT_NULL
#endif

#include <iostream>
#include <sstream>
#include <iomanip>

extern "C" void *DataBase( const void *conn )
{
  return reinterpret_cast<void *>(
    new MySQL::DataBase(
      *reinterpret_cast<const xxSQL::DataBase_connect *>(conn) ) );
}

namespace MySQL {

using std::string;
using std::vector;
using namespace std;

DataBase::DataBase( const xxSQL::DataBase_connect& c ) :
    xxSQL::DataBase( c ),
    thr( xmt::Thread::detached )
{
  con_cond.set( false );
  thr.launch( (xmt::Thread::entrance_type)conn_proc, (void *)this );
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
    mysql_close( _conn );
    _conn = 0;
    _flags = badbit;
  }

  // thr.join(); // no join, it detached
  //if ( thr.good() ) {
  //  thr.kill( SIGTERM );
  //}
}

int DataBase::conn_proc( void *p )
{
  DataBase *me = reinterpret_cast<DataBase *>(p);
  MYSQL* my = mysql_init( 0 );
  me->_conn = mysql_real_connect( my, me->_dbhost.length() > 0 ? me->_dbhost.c_str() : 0,
                            me->_dbusr.length() > 0 ? me->_dbusr.c_str() : 0,
                            me->_dbpass.length() > 0 ? me->_dbpass.c_str() : 0,
                            me->_dbname.length() > 0 ? me->_dbname.c_str() : 0,
                            0 /* _dbport 3306? */,
                            0 /* unix socket */,
                            0 /* me->_dbopt.length() > 0 */ );
  if ( me->_conn == 0 ) {
    me->_flags = badbit | failbit;
    mysql_close( my );
    me->_conn = 0;
  } else {
    me->_flags = goodbit;
  }
  me->con_cond.set( true );

  return 0;
}

void DataBase::reconnect()
{
  if ( _conn != 0 ) {
    mysql_close( _conn );
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
    thr.launch( (xmt::Thread::entrance_type)conn_proc, (void *)this );
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

  int _result = mysql_real_query( _conn, s.data(), s.length() );
  if ( _result != 0 ) {
    _flags = failbit;
    if ( _result == CR_COMMANDS_OUT_OF_SYNC ) {
      // mysql_free_result( ... );
    }
  }
}

void DataBase::exec( const char *s )
{
  if ( !good() ) {
    return;
  }

  int _result = mysql_query( _conn, s );
  if ( _result != 0 ) {
    _flags = failbit;
  }
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

const string My_time_now( "NOW()" );
const string My_IS_NULL( "ISNULL" );
const string My_IS_NOTNULL( "NOTNULL" );

// return quoted string
string DataBase::get_time() const
{
  return My_time_now;
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
  return My_IS_NULL;
}

string DataBase::IS_NOT_NULL() const
{
  return My_IS_NOTNULL;
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
  int _qresult = mysql_real_query( db->_conn, tmp.data(), tmp.length() );
  if ( _qresult != 0 ) {
    db->_flags = DataBase::failbit;
    return;
  }
  MYSQL_RES* _result = mysql_store_result( db->_conn );
  if ( _result == 0 ) {
    if ( mysql_field_count( db->_conn ) == 0 ) {
      // int nr = mysql_affected_rows( db->_conn );
    } else {
      db->_flags = DataBase::failbit;
    }
    return;
  }
  fields.clear();
  int n = mysql_num_fields( _result );
  fields.reserve( n );

  MYSQL_FIELD *f = mysql_fetch_fields( _result );
  for ( int i = 0; i < n; ++i ) {
    fields.push_back( f[i].name );
  }
  int ntuples = mysql_num_rows( _result );
  data.clear();
  data.reserve( ntuples );
  MYSQL_ROW row;
  for ( int j = 0; j < ntuples; ++j ) {
    row = mysql_fetch_row( _result );
    if ( row == 0 ) {
      break;
    }
    unsigned long *lengths = mysql_fetch_lengths( _result );
    data.push_back( datarow_type() );
    data[j].reserve( n );
    for ( int i = 0; i < n; ++i ) {
      data[j].push_back( row[i] == 0 ? "NULL" :  row[i] );
    }
  }
  mysql_free_result( _result );
}

} // namespace MySQL

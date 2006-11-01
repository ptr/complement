// -*- C++ -*- Time-stamp: <02/08/11 17:58:56 ptr>

/*
 * Copyright (c) 2002
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

#include <string>
#include <iostream>
#include <DB/xxSQL.h>

using namespace std;

int main( int argc, char * const *argv )
{
  try {
    xxSQL::DataBase_connect dbc;
    
    dbc.dbname( "func" );
    dbc.dbuser( "ptr");
    dbc.dberr( &std::cerr );

    xxSQL::DBxx db( xxSQL::DBxx::PostgreSQL, dbc );

    string rq( "CREATE TABLE t1 ( f1 INTEGER );" );

    db.begin_transaction();

    db.exec( rq );

    db.end_transaction();

    db.begin_transaction();

    rq = "INSERT INTO t1 VALUES ( 3 ); INSERT INTO t1 VALUES ( 4 );";
    db.exec( rq );

    db.end_transaction();

    db.begin_transaction();

    xxSQL::Cursor *cursor;
    rq = "SELECT * FROM t1;";
    cursor = db.create_cursor( rq.c_str() );
    cursor->fetch_all();
    int n = cursor->size();
    for ( int i = 0; i < n; ++i ) {
      cout << cursor->value( i, "f1" ) << endl;
    }
    db.delete_cursor( cursor );
  
    db.end_transaction();

    db.begin_transaction();
    rq = "DROP TABLE t1;";
    db.exec( rq );
    db.end_transaction();
    return 0;
  }
  catch ( ... ) {
    cerr << "Error" << endl;
  }
}

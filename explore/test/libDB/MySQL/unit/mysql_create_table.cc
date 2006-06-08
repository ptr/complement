// -*- C++ -*- Time-stamp: <06/06/07 09:34:42 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2004, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.0
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

#include <boost/test/unit_test.hpp>

#include <DB/xxSQL.h>
#include <sstream>
#include <iostream>

using namespace std;

void pg_create_table()
{
  try {
    xxSQL::DataBase_connect dbc;

    dbc.dbname( "ut" );
    dbc.dbuser( "" );
    dbc.dberr( &std::cerr );

    xxSQL::DBxx db( xxSQL::DBxx::MySQL, dbc );

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
    BOOST_CHECK( n == 2 );
    for ( int i = 0; i < n; ++i ) {
      stringstream s;
      s << (i + 3);
      BOOST_CHECK( cursor->value( i, "f1" ) == s.str() );
    }
    db.delete_cursor( cursor );

    db.end_transaction();

    db.begin_transaction();
    rq = "DROP TABLE t1;";
    db.exec( rq );
    db.end_transaction();
  }
  catch ( ... ) {
    cerr << "Error" << endl;
  }
}

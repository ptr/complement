// -*- C++ -*- Time-stamp: <01/07/19 18:08:26 ptr>

/*
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

#ifndef __DB_xxSQL_i_h
#define __DB_xxSQL_i_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$Id$"
#  else
#pragma ident "$Id$"
#  endif
#endif

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <vector>
#include <utility>

namespace xxSQL {

using namespace std;

class DataBase_connect
{
  public:
    DataBase_connect();

    void dbname( const char *nm )
      { name = nm; }
    void dbuser( const char *usr )
      { user = usr; }
    void dbpasswd( const char *passwd ) 
      { pass = passwd; }
    void dbhost( const char *h )
      { host = h; }
    void dbopt( const char *options )
      { opt = options; }
    void dbtty( const char *t )
      { tty = t; }
    void dbport( unsigned p )
      { port = p; }
    void dberr( std::ostream *e )
      { err = e; }

  private:
    string name;
    string user;
    string pass;
    string host;
    string opt;
    string tty;
    std::ostream *err;
    unsigned port;

    friend class DataBase;
    friend class DBxx;
};

class Cursor
{
  protected:
    Cursor( const char *nm );
    virtual ~Cursor();

  public:
    virtual void fetch_all() = 0;
    const string& value( int, const string& );
    const string& value( int, const char * );
    vector<vector<string> >::size_type size() const
      { return data.size(); }

  protected:
    string name;
    vector<string> fields;
    vector<vector<string> > data;

    friend class DataBase;
};

class DataBase
{
  public:
    enum {
      goodbit = 0,
      badbit  = 1,
      failbit = 2      
    };

    DataBase( const DataBase_connect& );

    virtual ~DataBase();

    virtual void reconnect() = 0;

    bool good() const
      { return _flags == goodbit; }
    bool bad() const
      { return (_flags & badbit) != 0; }
    bool fail() const
      { return (_flags & failbit) != 0; }
    void clear()
      { _flags = goodbit; }
    void clear( unsigned flags )
      { _flags &= ~flags; }
    virtual void exec( const string& ) = 0;
    virtual void exec( const char * ) = 0;

    virtual void begin_transaction() = 0;
    virtual void end_transaction() = 0;

    virtual Cursor *create_cursor( const char * ) = 0;
    void delete_cursor( Cursor *_c )
      { delete _c; }

    virtual string get_time() const = 0;
    virtual string get_time( time_t ) const = 0;
    virtual string IS_NULL() const = 0;
    virtual string IS_NOT_NULL() const = 0;

  protected:
    unsigned _flags;
    unsigned long _xc;
    string _dbname;
    string _dbusr;
    string _dbpass;
    string _dbhost;
    unsigned _dbport;
    string _dbopt;
    string _dbtty;
    std::ostream& _dberr;

    friend class Cursor;
};

} // namespace xxSQL

#endif // __DB_xxSQL_i_h

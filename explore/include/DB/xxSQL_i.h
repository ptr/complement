// -*- C++ -*- Time-stamp: <01/02/13 12:24:21 ptr>

/*
 *
 * Copyright (c) 1999-2000
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

#ifndef __DB_xxSQL_i_h
#define __DB_xxSQL_i_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
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

    DataBase( const char *name, const char *usr = 0, const char *passwd = 0,
              const char *host = 0, const char *port = 0, const char *opt = 0,
              const char *tty = 0, std::ostream *err = 0 );
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

// -*- C++ -*- Time-stamp: <00/06/02 18:16:14 ptr>

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

#ident "$SunId$"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <vector>
#include <utility>

namespace xxSQL {

class Cursor
{
  protected:
    Cursor( const char *nm );
    virtual ~Cursor();

  public:
    virtual void fetch_all() = 0;
    const __STD::string& value( int, const __STD::string& );
    const __STD::string& value( int, const char * );
    __STD::vector<__STD::vector<__STD::string> >::size_type size() const
      { return data.size(); }

  protected:
    __STD::string name;
    __STD::vector<__STD::string> fields;
    __STD::vector<__STD::vector<__STD::string> > data;

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
              const char *tty = 0 );
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
    virtual void exec( const __STD::string& ) = 0;
    virtual void exec( const char * ) = 0;

    virtual void begin_transaction() = 0;
    virtual void end_transaction() = 0;

    virtual Cursor *create_cursor( const char * ) = 0;
    void delete_cursor( Cursor *_c )
      { delete _c; }

  protected:
    unsigned _flags;
    unsigned long _xc;
    __STD::string _dbname;
    __STD::string _dbusr;
    __STD::string _dbpass;
    __STD::string _dbhost;
    unsigned _dbport;
    __STD::string _dbopt;
    __STD::string _dbtty;

    friend class Cursor;
};

} // namespace xxSQL

#endif // __DB_xxSQL_i_h

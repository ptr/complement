// -*- C++ -*- Time-stamp: <00/11/09 10:55:39 ptr>

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

#ifndef __PgSQL_h
#define __PgSQL_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

extern "C" {
  struct pg_conn;
} // extern "C"

#ifndef __xxSQL_i_h
#include <DB/xxSQL_i.h>
#endif

namespace PgSQL {

typedef ::pg_conn    DBconn;

class DataBase;
class Cursor;

struct NILL_type
{
};

extern NILL_type NILL;

class DataBase :
        public xxSQL::DataBase
{
  public:
    DataBase( const char *name, const char *usr = 0, const char *passwd = 0,
              const char *host = 0, const char *port = 0, const char *opt = 0,
              const char *tty = 0 );
    virtual ~DataBase();

    virtual void reconnect();
    virtual void exec( const __STD::string& );
    virtual void exec( const char * );

    virtual void begin_transaction();
    virtual void end_transaction();

    virtual xxSQL::Cursor *create_cursor( const char * );

    virtual std::string get_time() const;
    virtual std::string get_time( time_t ) const;
    virtual std::string IS_NULL() const;
    virtual std::string IS_NOT_NULL() const;

  private:
    DBconn *_conn;

    friend class Cursor;
};

class Cursor :
    public xxSQL::Cursor
{
  private:
    Cursor( const char *nm, DataBase *_db, const char *statement );
    virtual ~Cursor();

  public:
    virtual void fetch_all();

  private:
    DataBase *db;

    friend class DataBase;
};

} // namespace PgSQL


#endif // __PgSQL_h

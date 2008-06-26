// -*- C++ -*- Time-stamp: <02/08/04 15:35:55 ptr>

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

#ifndef __PgSQL_h
#define __PgSQL_h

extern "C" {
  struct pg_conn;
} // extern "C"

#ifndef __xxSQL_i_h
#include <DB/xxSQL_i.h>
#endif

#ifndef __XMT_H
#include <mt/xmt.h>
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
    DataBase( const xxSQL::DataBase_connect& );
    virtual ~DataBase();

    virtual void reconnect();
    virtual void exec( const std::string& );
    virtual void exec( const char * );

    virtual void begin_transaction();
    virtual void end_transaction();

    virtual xxSQL::Cursor *create_cursor( const char * );

    virtual std::string get_time() const;
    virtual std::string get_time( time_t ) const;
    virtual std::string IS_NULL() const;
    virtual std::string IS_NOT_NULL() const;

  private:
    static xmt::Thread::ret_t conn_proc( void * );

    DBconn *_conn;
    xmt::Thread thr;
    xmt::condition con_cond;

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

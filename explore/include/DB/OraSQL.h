// -*- C++ -*- Time-stamp: <00/10/17 09:47:29 ptr>

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

#ifndef __OraSQL_h
#define __OraSQL_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

extern "C" {
  struct OCIServer;
  struct OCISvcCtx;
  struct OCISession;
  struct OCIStmt;
} // extern "C"

#ifndef __xxSQL_i_h
#  include <DB/xxSQL_i.h>
#endif

namespace OraSQL {

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
              std::ostream *err = 0 );
    virtual ~DataBase();

    virtual void reconnect();
    virtual void exec( const __STD::string& );
    virtual void exec( const char * );

    virtual void begin_transaction();
    virtual void end_transaction();

    virtual xxSQL::Cursor *create_cursor( const char * );

  private:
    OCIServer  *_conn_srv;
    OCISvcCtx  *_conn_srv_ctx;
    OCISession *_conn_sess;

    class Init
    {
      public:
        Init();
        ~Init();

      private:
        static unsigned count;
    };

    int error_report( int, const char *s = 0 );
    std::ostream *_err;
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
    OCIStmt *_st;

    friend class DataBase;
};

} // namespace OraSQL

#endif // __OraSQL_h

// -*- C++ -*- Time-stamp: <00/10/12 10:13:48 ptr>

/*
 *
 * Copyright (c) 1999-2000
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

#ifndef __xxSQL_h
#define __xxSQL_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#ifndef __DB_xxSQL_i_h
#include <DB/xxSQL_i.h>
#endif

namespace xxSQL {

class DBxx
{
  public:
    enum DBvendor {
      PostgreSQL,
      Oracle8i
    };

    DBxx( DBvendor vendor,
          const char *name, const char *usr = 0, const char *passwd = 0,
          const char *host = 0, const char *port = 0, const char *opt = 0,
          const char *tty = 0 );
    ~DBxx();

    void reconnect()
      { _db->reconnect(); }

    bool good() const
      { return _db->good(); }
    bool bad() const
      { return _db->bad(); }
    bool fail() const
      { return _db->fail(); }
    void clear()
      { _db->clear(); }
    void exec( const __STD::string& query )
      { _db->exec( query ); }
    void exec( const char *query )
      { _db->exec( query ); }

    void begin_transaction()
      { _db->begin_transaction(); }
    void end_transaction()
      { _db->end_transaction(); }

    Cursor *create_cursor( const char *query )
      { return _db->create_cursor( query ); }
    void delete_cursor( Cursor *cursor )
      { _db->delete_cursor( cursor ); }

  private:
    DataBase *_db;
};

} // namespace xxSQL

#endif // __xxSQL_h

// -*- C++ -*- Time-stamp: <02/08/23 14:12:49 ptr>

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

#ifndef __xxSQL_h
#define __xxSQL_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

#ifndef __DB_xxSQL_i_h
#include <DB/xxSQL_i.h>
#endif

#include <iostream>

namespace xxSQL {

class DBxx
{
  public:
    enum DBvendor {
      Unknown,
      PostgreSQL,
      Oracle8i
    };

    enum flags_t {
      truss = 0x1
    };

    DBxx( DBvendor vendor, const DataBase_connect& );
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
    void clear( unsigned flags )
      { _db->clear( flags ); }
    void exec( const std::string& query )
      { 
        if ( (fmtflags & truss) != 0 ) {
          std::cerr << query << std::endl;
        }
        _db->exec( query );
      }
    void exec( const char *query )
      {
        if ( (fmtflags & truss) != 0 ) {
          std::cerr << query << std::endl;
        }
        _db->exec( query );
      }

    void begin_transaction()
      { _db->begin_transaction(); }
    void end_transaction()
      { _db->end_transaction(); }

    Cursor *create_cursor( const char *query )
      {
        if ( (fmtflags & truss) != 0 ) {
          std::cerr << query << std::endl;
        }
        return _db->create_cursor( query );
      }
    void delete_cursor( Cursor *cursor )
      { _db->delete_cursor( cursor ); }

    string escape_data( const std::string& s ) const;
    string escape_data( const char* s ) const;
    string get_time() const;
    string get_time( time_t ) const;
    string IS_NULL() const;
    string IS_NOT_NULL() const;
    DBvendor vendor() const
      { return _dbvendor; }

    void setf( unsigned _f )
      { fmtflags |= _f; }
    void unsetf( unsigned _f )
      { fmtflags &= ~_f; }

  private:
    DataBase *_db;
    DBvendor _dbvendor;
    unsigned fmtflags;
    void *lh; // DB wrapper library handler
};

} // namespace xxSQL

#endif // __xxSQL_h

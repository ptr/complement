// -*- C++ -*- Time-stamp: <00/05/23 16:14:26 ptr>

#ifndef __PgSQL_h
#define __PgSQL_h

#ident "$SunId$"

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
    virtual void delete_cursor( xxSQL::Cursor * );

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

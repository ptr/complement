// -*- C++ -*- Time-stamp: <00/05/23 16:15:16 ptr>

#ident "$SunId$"

#include <config/feature.h>

#include "DB/xxSQL.h"
#include "DB/PgSQL.h"

namespace xxSQL {

DBxx::DBxx( DBvendor vendor,
            const char *name, const char *usr, const char *passwd,
            const char *host, const char *port, const char *opt,
            const char *tty )
{
  switch ( vendor ) {
    case PostgreSQL:
      _db = new PgSQL::DataBase(name,usr,passwd,host,port,opt,tty);
      break;
    default:
      _db = 0;
      break;
  }
}

DBxx::~DBxx()
{
  delete _db;
}

} // namespace xxSQL

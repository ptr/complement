// -*- C++ -*- Time-stamp: <00/10/16 17:46:22 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#include <config/feature.h>

#include "DB/xxSQL.h"
#ifdef __DB_POSTGRES
#  include "DB/PgSQL.h"
#endif
#ifdef __DB_ORACLE
#  include "DB/OraSQL.h"
#endif

namespace xxSQL {

DBxx::DBxx( DBvendor vendor,
            const char *name, const char *usr, const char *passwd,
            const char *host, const char *port,
            std::ostream *err,
            const char *opt,
            const char *tty )
{
  switch ( vendor ) {
    case PostgreSQL:
#ifdef __DB_POSTGRES
      _db = new PgSQL::DataBase(name,usr,passwd,host,port,opt,tty);
#else
      _db = 0;
#endif
      break;
    case Oracle8i:
#ifdef __DB_ORACLE
      _db = new OraSQL::DataBase(name,usr,passwd,err);
#else
      _db = 0;
#endif
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

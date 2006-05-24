// -*- C++ -*- Time-stamp: <01/07/19 15:42:36 ptr>

/*
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
// #ifdef __DB_ORACLE

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$Id$"
#  else
#pragma ident "$Id$"
#  endif
#endif

#include <config/feature.h>
#include "DB/OraSQL.h"

#include <cstdio>
#include <oci.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <list>
#include <mt/xmt.h>

namespace OraSQL {

using std::string;
using std::vector;
using namespace std;

OCIEnv    *ora_env;
OCIError  *ora_err;

char new_buff[64];
unsigned DataBase::Init::count = 0;
__impl::Mutex init_lock;

DataBase::Init::Init()
{
  MT_REENTRANT( init_lock, __x1 );
  if ( count++ == 0 ) {
    OCIEnvCreate( &ora_env, OCI_THREADED, 0, 0, 0, 0, 0, 0 );
    OCIHandleAlloc( ora_env, (void **)&ora_err, OCI_HTYPE_ERROR, 0, 0 );
  }
}

DataBase::Init::~Init()
{
  MT_REENTRANT( init_lock, __x1 );
  if ( --count == 0 ) {
    OCIHandleFree( ora_err, OCI_HTYPE_ERROR );
    OCIHandleFree( ora_env, OCI_HTYPE_ENV );
  }
}

int DataBase::error_report( int err, const char *s )
{
  if ( _err == 0 ) {
    return 0;
  }

  unsigned char buf[512];
  int errcode = 0;

  int ret = 0;

  switch ( err ) {
    case OCI_SUCCESS:
      break;
    case OCI_SUCCESS_WITH_INFO:
      *_err << "Oracle Error - OCI_SUCCESS_WITH_INFO" << endl;
      if ( s != 0 ) {
        *_err << "  Statement was: " << s << endl;
      }
      ret = -1;
      break;
    case OCI_NEED_DATA:
      *_err << "Oracle Error - OCI_NEED_DATA" << endl;
      if ( s != 0 ) {
        *_err << "  Statement was: " << s << endl;
      }
      ret = -1;
      break;
    case OCI_NO_DATA:
      *_err << "Oracle Error - OCI_NODATA" << endl;
      if ( s != 0 ) {
        *_err << "  Statement was: " << s << endl;
      }
      ret = -1;
      break;
    case OCI_ERROR:
      OCIErrorGet( ora_err, 1, 0, &errcode, buf, sizeof(buf),
                   OCI_HTYPE_ERROR);
      *_err << "Oracle Error - " << buf;
      if ( s != 0 ) {
        *_err << "  Statement was: " << s;
      }
      *_err << endl;
      ret = -1;
      break;
    case OCI_INVALID_HANDLE:
      *_err << "Oracle Error - OCI_INVALID_HANDLE" << endl;
      if ( s != 0 ) {
        *_err << "  Statement was: " << s << endl;
      }
      ret = -1;
      break;
    case OCI_STILL_EXECUTING:
      *_err << "Oracle Error - OCI_STILL_EXECUTE" << endl;
      if ( s != 0 ) {
        *_err << "  Statement was: " << s << endl;
      }
      ret = -1;
      break;
    case OCI_CONTINUE:
      *_err << "Oracle Error - OCI_CONTINUE" << endl;
      if ( s != 0 ) {
        *_err << "  Statement was: " << s << endl;
      }
      ret = -1;
      break;
    default:
      break;
  }

  return ret;
}

DataBase::DataBase( const char *name, const char *usr, const char *passwd,
                    std::ostream *err_stream ) :
    xxSQL::DataBase( name, usr, passwd ),
    _conn_sess( 0 ),
    _err( err_stream )
{
  new (new_buff) Init();

  OCIHandleAlloc( ora_env, (void **)&_conn_srv, OCI_HTYPE_SERVER, 0, 0 );
  OCIHandleAlloc( ora_env, (void **)&_conn_srv_ctx, OCI_HTYPE_SVCCTX, 0, 0 );

  /*
  OCILogon( ora_env, ora_err, &ora_srv_ctx,
            usr, strlen(usr), passwd, strlen(passwd), name, strlen( name ) );
  */

  OCIServerAttach( _conn_srv, ora_err, (unsigned char *)name, strlen(name), OCI_DEFAULT );
  OCIAttrSet( _conn_srv_ctx, OCI_HTYPE_SVCCTX, _conn_srv, 0, OCI_ATTR_SERVER, ora_err );
  OCIHandleAlloc( ora_env, (void **)&_conn_sess, OCI_HTYPE_SESSION, 0, 0 );
  OCIAttrSet( _conn_sess, OCI_HTYPE_SESSION, (unsigned char *)usr, strlen(usr),
              OCI_ATTR_USERNAME, ora_err );
  OCIAttrSet( _conn_sess, OCI_HTYPE_SESSION, (unsigned char *)passwd, strlen(passwd),
              OCI_ATTR_PASSWORD, ora_err );
  int err =
    OCISessionBegin( _conn_srv_ctx, ora_err, _conn_sess, OCI_CRED_RDBMS, OCI_DEFAULT );

  error_report( err );

  OCIAttrSet( _conn_srv_ctx, OCI_HTYPE_SVCCTX, _conn_sess, 0, OCI_ATTR_SESSION, ora_err );

  if ( err != OCI_SUCCESS ) {
    _flags = badbit | failbit;
    OCISessionEnd( _conn_srv_ctx, ora_err, _conn_sess, OCI_DEFAULT );
    OCIHandleFree( _conn_sess, OCI_HTYPE_SESSION ); // -- ?
    OCIServerDetach( _conn_srv, ora_err, OCI_DEFAULT );
    OCIHandleFree( _conn_srv, OCI_HTYPE_SERVER );
    OCIHandleFree( _conn_srv_ctx, OCI_HTYPE_SVCCTX );
  } else {
    _flags = goodbit;
  }

  // Change date format (for current session).
  OCIStmt *ora_state;
  OCIHandleAlloc( ora_env, (void **)&ora_state, OCI_HTYPE_STMT, 0, 0 );

  string s( "ALTER SESSION SET NLS_DATE_FORMAT = 'YYYY-MM-DD HH24:MI:SS'" );
  err = OCIStmtPrepare( ora_state, ora_err, (unsigned char *)s.c_str(), s.size(),
                        OCI_NTV_SYNTAX, OCI_DEFAULT );
  error_report( err );
  err = OCIStmtExecute( _conn_srv_ctx, ora_state, ora_err, 1, 0, 0, 0,
                        OCI_DEFAULT | OCI_COMMIT_ON_SUCCESS );
  error_report( err );

  if ( err != OCI_SUCCESS ) {
    _flags = failbit;
  }
  OCIHandleFree( ora_state, OCI_HTYPE_STMT );
}

DataBase::DataBase( const xxSQL::DataBase_connect& c ) :
    xxSQL::DataBase( c ),
    _conn_sess( 0 ),
    _err( err_stream )
{
  new (new_buff) Init();

  OCIHandleAlloc( ora_env, (void **)&_conn_srv, OCI_HTYPE_SERVER, 0, 0 );
  OCIHandleAlloc( ora_env, (void **)&_conn_srv_ctx, OCI_HTYPE_SVCCTX, 0, 0 );

  /*
  OCILogon( ora_env, ora_err, &ora_srv_ctx,
            usr, strlen(usr), passwd, strlen(passwd), name, strlen( name ) );
  */

  OCIServerAttach( _conn_srv, ora_err, (unsigned char *)name, strlen(name), OCI_DEFAULT );
  OCIAttrSet( _conn_srv_ctx, OCI_HTYPE_SVCCTX, _conn_srv, 0, OCI_ATTR_SERVER, ora_err );
  OCIHandleAlloc( ora_env, (void **)&_conn_sess, OCI_HTYPE_SESSION, 0, 0 );
  OCIAttrSet( _conn_sess, OCI_HTYPE_SESSION, (unsigned char *)usr, strlen(usr),
              OCI_ATTR_USERNAME, ora_err );
  OCIAttrSet( _conn_sess, OCI_HTYPE_SESSION, (unsigned char *)passwd, strlen(passwd),
              OCI_ATTR_PASSWORD, ora_err );
  int err =
    OCISessionBegin( _conn_srv_ctx, ora_err, _conn_sess, OCI_CRED_RDBMS, OCI_DEFAULT );

  error_report( err );

  OCIAttrSet( _conn_srv_ctx, OCI_HTYPE_SVCCTX, _conn_sess, 0, OCI_ATTR_SESSION, ora_err );

  if ( err != OCI_SUCCESS ) {
    _flags = badbit | failbit;
    OCISessionEnd( _conn_srv_ctx, ora_err, _conn_sess, OCI_DEFAULT );
    OCIHandleFree( _conn_sess, OCI_HTYPE_SESSION ); // -- ?
    OCIServerDetach( _conn_srv, ora_err, OCI_DEFAULT );
    OCIHandleFree( _conn_srv, OCI_HTYPE_SERVER );
    OCIHandleFree( _conn_srv_ctx, OCI_HTYPE_SVCCTX );
  } else {
    _flags = goodbit;
  }

  // Change date format (for current session).
  OCIStmt *ora_state;
  OCIHandleAlloc( ora_env, (void **)&ora_state, OCI_HTYPE_STMT, 0, 0 );

  string s( "ALTER SESSION SET NLS_DATE_FORMAT = 'YYYY-MM-DD HH24:MI:SS'" );
  err = OCIStmtPrepare( ora_state, ora_err, (unsigned char *)s.c_str(), s.size(),
                        OCI_NTV_SYNTAX, OCI_DEFAULT );
  error_report( err );
  err = OCIStmtExecute( _conn_srv_ctx, ora_state, ora_err, 1, 0, 0, 0,
                        OCI_DEFAULT | OCI_COMMIT_ON_SUCCESS );
  error_report( err );

  if ( err != OCI_SUCCESS ) {
    _flags = failbit;
  }
  OCIHandleFree( ora_state, OCI_HTYPE_STMT );
}

DataBase::~DataBase()
{
  if ( _flags == goodbit ) {
    OCISessionEnd( _conn_srv_ctx, ora_err, _conn_sess, OCI_DEFAULT );
    OCIHandleFree( _conn_sess, OCI_HTYPE_SESSION ); // -- ?
    OCIServerDetach( _conn_srv, ora_err, OCI_DEFAULT );
    OCIHandleFree( _conn_srv, OCI_HTYPE_SERVER );
    OCIHandleFree( _conn_srv_ctx, OCI_HTYPE_SVCCTX );

    _flags = badbit;
  }
  ((Init *)new_buff)->~Init();
}

void DataBase::reconnect()
{
  if ( _flags == goodbit ) {
    return;
  }

  OCISessionEnd( _conn_srv_ctx, ora_err, _conn_sess, OCI_DEFAULT );
  OCIHandleFree( _conn_sess, OCI_HTYPE_SESSION ); // -- ?
  OCIServerDetach( _conn_srv, ora_err, OCI_DEFAULT );
  OCIHandleFree( _conn_srv, OCI_HTYPE_SERVER );
  OCIHandleFree( _conn_srv_ctx, OCI_HTYPE_SVCCTX );    

  OCIHandleAlloc( ora_env, (void **)&_conn_srv, OCI_HTYPE_SERVER, 0, 0 );
  OCIHandleAlloc( ora_env, (void **)&_conn_srv_ctx, OCI_HTYPE_SVCCTX, 0, 0 );

  /*
  OCILogon( ora_env, ora_err, &ora_srv_ctx,
            usr, strlen(usr), passwd, strlen(passwd), name, strlen( name ) );
  */

  OCIServerAttach( _conn_srv, ora_err, (unsigned char *)_dbname.c_str(), _dbname.size(), OCI_DEFAULT );
  OCIAttrSet( _conn_srv_ctx, OCI_HTYPE_SVCCTX, _conn_srv, 0, OCI_ATTR_SERVER, ora_err );
  OCIHandleAlloc( ora_env, (void **)&_conn_sess, OCI_HTYPE_SESSION, 0, 0 );
  OCIAttrSet( _conn_sess, OCI_HTYPE_SESSION, (unsigned char *)_dbusr.c_str(), _dbusr.size(),
              OCI_ATTR_USERNAME, ora_err );
  OCIAttrSet( _conn_sess, OCI_HTYPE_SESSION, (unsigned char *)_dbpass.c_str(), _dbpass.size(),
              OCI_ATTR_PASSWORD, ora_err );
  int err =
    OCISessionBegin( _conn_srv_ctx, ora_err, _conn_sess, OCI_CRED_RDBMS, OCI_DEFAULT );
  error_report( err );
  OCIAttrSet( _conn_srv_ctx, OCI_HTYPE_SVCCTX, _conn_sess, 0, OCI_ATTR_SESSION, ora_err );

  if ( err != OCI_SUCCESS ) {
    _flags = badbit | failbit;
    OCISessionEnd( _conn_srv_ctx, ora_err, _conn_sess, OCI_DEFAULT );
    OCIHandleFree( _conn_sess, OCI_HTYPE_SESSION ); // -- ?
    OCIServerDetach( _conn_srv, ora_err, OCI_DEFAULT );
    OCIHandleFree( _conn_srv, OCI_HTYPE_SERVER );
    OCIHandleFree( _conn_srv_ctx, OCI_HTYPE_SVCCTX );
  } else {
    _flags = goodbit;
  }

  // Change date format (for current session).
  OCIStmt *ora_state;
  OCIHandleAlloc( ora_env, (void **)&ora_state, OCI_HTYPE_STMT, 0, 0 );

  string s( "ALTER SESSION SET NLS_DATE_FORMAT = 'YYYY-MM-DD HH24:MI:SS'" );
  err = OCIStmtPrepare( ora_state, ora_err, (unsigned char *)s.c_str(), s.size(),
                        OCI_NTV_SYNTAX, OCI_DEFAULT );
  error_report( err, s.c_str() );
  err = OCIStmtExecute( _conn_srv_ctx, ora_state, ora_err, 1, 0, 0, 0,
                        OCI_DEFAULT | OCI_COMMIT_ON_SUCCESS );
  error_report( err, s.c_str() );

  if ( err != OCI_SUCCESS ) {
    _flags = failbit;
  }
  OCIHandleFree( ora_state, OCI_HTYPE_STMT );
}

void DataBase::exec( const string& s )
{
  if ( !good() ) {
    return;
  }

  OCIStmt *ora_state;
  OCIHandleAlloc( ora_env, (void **)&ora_state, OCI_HTYPE_STMT, 0, 0 );

  int err =
    OCIStmtPrepare( ora_state, ora_err, (unsigned char *)s.c_str(), s.size(),
                    OCI_NTV_SYNTAX, OCI_DEFAULT );
  error_report( err, s.c_str() );
  err = OCIStmtExecute( _conn_srv_ctx, ora_state, ora_err, 1, 0, 0, 0, OCI_DEFAULT );
  //                                                                  | OCI_COMMIT_ON_SUCCESS
  error_report( err, s.c_str() );
  if ( err != OCI_SUCCESS ) {
    _flags = failbit;
  }
  OCIHandleFree( ora_state, OCI_HTYPE_STMT );
}

void DataBase::exec( const char *s )
{
  if ( !good() ) {
    return;
  }

  OCIStmt *ora_state;
  OCIHandleAlloc( ora_env, (void **)&ora_state, OCI_HTYPE_STMT, 0, 0 );

  int err =
    OCIStmtPrepare( ora_state, ora_err, (unsigned char *)s, strlen( s ),
                    OCI_NTV_SYNTAX, OCI_DEFAULT );
  error_report( err, s );

  err = OCIStmtExecute( _conn_srv_ctx, ora_state, ora_err, 0, 0, 0, 0, OCI_DEFAULT );
  error_report( err, s );
  if ( err != OCI_SUCCESS ) {
    _flags = failbit;
  }
  OCIHandleFree( ora_state, OCI_HTYPE_STMT );
}

void DataBase::begin_transaction()
{
//  cerr << "DataBase::begin_transaction begin" << endl;
//  exec( "BEGIN" );
//  cerr << "DataBase::begin_transaction end" << endl;
}

void DataBase::end_transaction()
{
//  cerr << "DataBase::end_transaction begin" << endl;
//  exec( "COMMIT" );
  int err = OCITransCommit( _conn_srv_ctx, ora_err, 0 );
  error_report( err, "transaction commit" );
//  cerr << "DataBase::end_transaction end" << endl;
}

xxSQL::Cursor *DataBase::create_cursor( const char *str )
{
  ostringstream s;

  s << "CU" << hex << _xc++;
 
  return new Cursor( s.str().c_str(), this, str );
}

const string Ora_IS_NULL( "IS NULL" );
const string Ora_IS_NOTNULL( "IS NOT NULL" );

// return quoted string
string DataBase::get_time() const
{
  // Oracle has 'sysdate' function, but when I use it via OCI, it store
  // time with 8 hours before current time
  return get_time( time(0) );
}

// return quoted string
string DataBase::get_time( time_t t ) const
{
  ostringstream os;
  tm tstr;
  localtime_r( &t, &tstr );
  os << '\'' << (tstr.tm_year + 1900)
     << '-' << setw( 2 ) << setfill( '0' ) << (tstr.tm_mon+1)
     << '-' << setw( 2 ) << setfill( '0' ) << tstr.tm_mday
     << ' ' << setw( 2 ) << setfill( '0' ) << tstr.tm_hour
     << ':' << setw( 2 ) << setfill( '0' ) << tstr.tm_min
     << ':' << setw( 2 ) << setfill( '0' ) << tstr.tm_sec
     << '\'';
  return os.str();
}

string DataBase::IS_NULL() const
{
  return Ora_IS_NULL;
}

string DataBase::IS_NOT_NULL() const
{
  return Ora_IS_NOTNULL;
}

Cursor::Cursor( const char *nm, DataBase *_db, const char *s ) :
    xxSQL::Cursor( nm ),
    db( _db )
{
  OCIHandleAlloc( ora_env, (void **)&_st, OCI_HTYPE_STMT, 0, 0 );
  int err =
    OCIStmtPrepare( _st, ora_err, (unsigned char *)s, strlen( s ),
                    OCI_NTV_SYNTAX, OCI_DEFAULT );
  if ( db ) {
    db->error_report( err, s );
  }

  // string tmp = "DECLARE ";
  // tmp += nm;
  // tmp += " CURSOR FOR ";
  // tmp += s;

  // db->exec( tmp );
}

Cursor::~Cursor()
{
  OCIHandleFree( _st, OCI_HTYPE_STMT );
  // string tmp = "CLOSE ";
  // tmp += name;
  // db->exec( tmp );
}

#define ORACLE_FILD_BUFFER_LIMIT  4000

struct oracle_fields
{
    OCIDefine *def;   
//    unsigned short type;
    char buff[ORACLE_FILD_BUFFER_LIMIT];
};

string lower( const string& s )
{
  string l;
  l.reserve( s.length() );
  for ( string::const_iterator i = s.begin(); i != s.end(); ++i ) {
    l += tolower( *i );
  }
  return l;
}

string lower( const char *s )
{
  string l;
  unsigned n = strlen(s);
  const char *e = s + n;
  l.reserve( n );
  while ( s != e ) {
    l += tolower( *s++ );
  }
  return l;
}

void Cursor::fetch_all()
{
  fields.clear();
  data.clear();

  int err = OCIStmtExecute( db->_conn_srv_ctx, _st, ora_err, 0, 0, 0, 0, OCI_DEFAULT );
  if ( db ) {
    db->error_report( err, "fetch all" );
  }
  if ( err != OCI_SUCCESS ) {
    db->_flags = DataBase::failbit;
    return;
  }

  int n = 0;

  OCIParam *ora_param;
  unsigned cnt = 1;
  // unsigned short data_type;
  unsigned char *col;
  unsigned col_len;

  unsigned length;
  std::list<oracle_fields> flist;
  oracle_fields *last_fields;

  while ( OCIParamGet( _st, OCI_HTYPE_STMT, ora_err,
                       (void **)&ora_param, cnt++ ) == OCI_SUCCESS ) {
    flist.push_back( oracle_fields() );
    last_fields = &(flist.back());
    
    // OCIAttrGet( ora_param, OCI_DTYPE_PARAM, &last_fields->type, 0, OCI_ATTR_DATA_TYPE, ora_err );
    // ora_error( cerr, ora_err, err );
    OCIAttrGet( ora_param, OCI_DTYPE_PARAM, &col, &col_len, OCI_ATTR_NAME, ora_err );
    // OCIAttrGet( ora_param, OCI_DTYPE_PARAM, &length, 0, OCI_ATTR_DATA_SIZE, ora_err );
    // The call above return in length fantastic value 1441792 in any case,
    // so I will ignore it.
    // ora_error( cerr, ora_err, err );
    fields.push_back( lower((char *)col) );
    ++n;
    OCIDefineByPos( _st, &last_fields->def, ora_err, n, &last_fields->buff,
                    ORACLE_FILD_BUFFER_LIMIT,
                    SQLT_STR, 0, 0, 0, OCI_DEFAULT );
  }

  int j = 0;
  std::list<oracle_fields>::iterator k;
  while ( OCIStmtFetch( _st, ora_err, 1, OCI_FETCH_NEXT, OCI_DEFAULT ) != OCI_NO_DATA ) {
    data.push_back();
    data[j].reserve( n );
    k = flist.begin();
    for ( int i = 0; i < n; ++i ) {
      data[j].push_back( (*k++).buff );
    }
    ++j;
  }
}

} // namespace OraSQL

// #endif // __DB_ORACLE

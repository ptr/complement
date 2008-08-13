// -*- C++ -*- Time-stamp: <08/08/13 13:28:22 yeti>

/*
 * Copyright (c) 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#include "sc_perf_suite.h"
#include <vector>

#include <mt/thread>
#include <mt/mutex>

#include <db_cxx.h>
#include <sys/stat.h>

using namespace std;

int EXAM_IMPL(sleepycat_test::hash_open)
{
  DbEnv env( 0 );
  const char logDir[] = "./DB_logs";

  mkdir( logDir, 0777 );

  env.set_lg_dir( logDir );
  env.set_flags(DB_AUTO_COMMIT | DB_LOG_INMEMORY | DB_LOG_AUTOREMOVE, 1);
  env.open( ".", DB_INIT_TXN | DB_INIT_LOG | DB_CREATE | DB_INIT_MPOOL, 0600);

  Db database( &env, 0 );

  // This should suffice for most users.      
  database.set_h_nelem(30);
  // The BDB documentation says the fill factor should be 
  // (pagesize - 32) / (average_key_size + average_data_size + 8);
  // For labels, average key size = sizeof(TId) = 16, average data size = 0,
  // supposing 4K pages this gives us 169 1/3 for the fill factor.   
  database.set_h_ffactor(150);
  // For opening in the environment we should use a relative path.
  database.open( 0, "tmp.db", 0, DB_HASH, DB_CREATE, 0600);

  database.close( 0 );

  unlink( "tmp.db" );
  unlink( "__db.001" );
  unlink( "__db.002" );
  unlink( "__db.003" );
  unlink( "__db.004" );
  unlink( "__db.005" );

  rmdir( logDir );

  return 0;
}

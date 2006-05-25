
#include <db.h>
#include <iostream>
#include <iomanip>

using namespace std;

int main()
{
  DB *DBref;
  DB_ENV *DBenv;

  int err = 0;

  err = db_env_create( &DBenv, 0 );

  if ( err ) {
    cerr << __FILE__ << ":" << __LINE__ << " " << err << endl;
  }

  DBenv->set_tmp_dir( DBenv, "./dbtmp" );
  err = DBenv->open( DBenv, "./dbdir", /* DB_INIT_LOCK | */ DB_INIT_CDB | DB_INIT_MPOOL | DB_CREATE | DB_THREAD, 0 );

  if ( err ) {
    cerr << __FILE__ << ":" << __LINE__ << " " << err << endl;
  }

  err = db_create( &DBref, DBenv, 0 );

  if ( err ) {
    cerr << __FILE__ << ":" << __LINE__ << " " << err << endl;
  }

  err = DBref->open( DBref, 0, "sample.db", 0, DB_BTREE, DB_CREATE | DB_THREAD, 0664 );

  if ( err ) {
    cerr << __FILE__ << ":" << __LINE__ << " " << err << endl;
  }

  err = DBref->close( DBref, 0 );

  if ( err ) {
    cerr << __FILE__ << ":" << __LINE__ << " " << err << endl;
  }

  DBenv->close( DBenv, 0 );

  if ( err ) {
    cerr << __FILE__ << ":" << __LINE__ << " " << err << endl;
  }

  return 0;
}

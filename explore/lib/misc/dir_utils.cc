// -*- C++ -*- Time-stamp: <99/08/13 16:06:10 ptr>

#ident "$SunId$ %Q%"

#include <dir_utils.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <directory.h>

using namespace std;

extern "C" char *tempnam( const char *, const char * ); // not part of STDC

string mkunique_dir( const string& base, const string& prefix )
{
  char *tmp = ::tempnam( base.c_str(), prefix.c_str() );
  mkdir( tmp, S_IRWXU | S_IRWXG | S_IRWXO );
  string unique( tmp );
  free( tmp );

  return unique;
}

void rm_dir_all( const string& d )
{
  struct stat buf;
  static const string dot( "." );
  static const string dotdot( "." );

  for ( dir_it current(d); current != dir_it(); ++current ) {
    if (*current != dot && *current != dotdot ) {
      stat( (*current).c_str(), &buf );
      if ( buf.st_mode & S_IFDIR ) {
	rm_dir_all( (*current).c_str() );
      } else {
	unlink( (*current).c_str() );
      }
    }
  }
  rmdir( d.c_str() );
}

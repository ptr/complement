// -*- C++ -*- Time-stamp: <98/01/16 14:37:25 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include "dir_utils.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "directory.h"

string mkunique_dir( const string& base, const string& prefix )
{
  static const string XXXXXX( "XXXXXX" );
  string unique( base );
  if ( unique.length() > 0 && unique.at( unique.length()-1 ) != '/' ) {
    unique += '/';
  }
  unique += prefix;
  unique += XXXXXX;
  // not correct, but I know implementation,
  // and length will be same.
  mktemp( const_cast<char *>(unique.c_str()) );
  mkdir( unique.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );

  return unique;
}

void rm_dir_all( const string& d )
{
  struct stat buf;

  for ( dir_it current(d); current != dir_it(); ++current ) {
    if (*current != "." && *current != ".." ) {
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

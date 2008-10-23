// -*- C++ -*- Time-stamp: <05/02/22 22:34:53 ptr>

#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

// #include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int main( int, char * const * )
{
#if 1
  namespace fs = boost::filesystem;

  fs::path full_path( fs::initial_path() );

  // string p = ".";
  string p = "/tmp";
  // string p = "/opt/mail-coll";
  full_path = fs::system_complete( fs::path( p, fs::native ) );

  if ( !fs::exists( full_path ) ) {
    cerr << "\nNot found: " << full_path.native_file_string() << endl;
    return 1;
  }
  if ( fs::is_directory( full_path ) ) {
    fs::directory_iterator end_iter;
    for ( fs::directory_iterator di( full_path ); di != end_iter; ++di ) {
      try {
        if ( !fs::is_directory( *di ) ) {
//          string ext = ".cc";
//          string nm = di->leaf();
//          string::size_type pos = nm.rfind( ext );
//          if ( pos != string::npos && (pos == (nm.size() - 3)) ) {
//            cerr << nm << endl;
//          }
          struct stat buf;
          if ( stat( di->string().c_str(), &buf ) == 0 ) {
            cout << di->leaf() << ' ' << buf.st_size << ' ' << buf.st_mtime << endl;
          } else {
            cerr << "Problems with " << di->leaf() << endl;
          }
        } else {
          cerr << di->leaf()<< " [directory]\n";
        }
      }
      catch ( const std::exception& ex ) {
        cerr << di->leaf() << " " << ex.what() << endl;
      }
    }
  }
#else
  DIR *d = opendir( "." );
  closedir( d );
#endif

  return 0;
}

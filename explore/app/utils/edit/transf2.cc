// -*- C++ -*- Time-stamp: <04/06/15 16:05:52 ptr>

#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>

// #include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <vector>

using namespace std;
using namespace boost;

regex REmf( "MAIL FROM:[[:space:]]*(.*)" );
regex RErt( "RCPT TO:[[:space:]]*(.*)" );
regex REadr( "[[:word:]\\-][[:word:]\\.\\-]*@[[:word:]][[:word:]\\.\\-]*" );

struct rec
{
    string name;
    unsigned long size;
    unsigned long time;
    unsigned long count;
    string MailFrom;
    string RcptTo;

    rec& operator =( const rec& r )
      {
        name = r.name;
        size = r.size;
        time = r.time;
        count = r.count;
        MailFrom = r.MailFrom;
        RcptTo = r.RcptTo;
        return *this;
      }

    rec& operator =( unsigned long c )
      {
        count = c;
        return *this;
      }
};

bool less_tm( const rec& r1,  const rec& r2 )
{ return r1.time < r2.time; }

class incr
{
  public:
    incr() :
        _count(0)
      { }

    unsigned long operator ()()
      { return ++_count; }

  private:
    unsigned long _count;
};

void out( const rec& r1 )
{
  static unsigned long time_prev = 0;
  cout << "# ----- begin\n"
       << "delay " << (time_prev == 0? 0 : (r1.time - time_prev)) << "\n"
       << "# size " << r1.size << '\n'
       << "# count " << r1.count << '\n'
       << "connect $1 $2\n"
       << "expected 220\n"
       << "EHLO $3\n"
       << "expected 250\n"
       << "MAIL FROM:<" << r1.MailFrom << ">\n"
       << "expected 250\n";

  smatch m;
  if ( regex_search( r1.RcptTo, m, REadr ) ) {
    cout << "RCPT TO:<" << m.str() << ">\n"
         << "expected 250\n";
    string tmp = m.suffix().str();
    while ( regex_search( const_cast<const string&>(tmp).begin(), const_cast<const string&>(tmp).end(), m, REadr ) ) {
      cout << "RCPT TO:<" << m.str() << ">\n"
           << "expected 250\n";
      
      tmp = m.suffix().str();
    }
  } else {
    cerr << "Bad address: " << r1.RcptTo << endl;
  }

  cout << "send $6 $7\n"
       << "DATA\n"
       << "expected 354\n"
       << "include /opt/mcollection2/x2/" << r1.name << "\n"
       << "expected 250\n"
       << "wait\n"
       << "QUIT\n"
       << "expected 221\n"
       << "disconnect\n"
       << "# ----- end\n";

  time_prev = r1.time;
}

int main( int, char * const * )
{
  namespace fs = boost::filesystem;

  vector<rec> v;

  fs::path full_path( fs::initial_path() );

  // string p = ".";
  string p = "/opt/mcollection2/arch_in";
  // string p = "/opt/mail-coll";
  full_path = fs::system_complete( fs::path( p, fs::native ) );

  if ( !fs::exists( full_path ) ) {
    cerr << "\nNot found: " << full_path.native_file_string() << endl;
    return 1;
  }
  if ( fs::is_directory( full_path ) ) {
    unsigned long time_prev = 0;
    fs::directory_iterator end_iter;
    for ( fs::directory_iterator di( full_path ); di != end_iter; ++di ) {
      try {
        if ( !fs::is_directory( *di ) ) {
          struct stat buf;
          if ( stat( di->string().c_str(), &buf ) == 0 ) {
            // cout << di->leaf() << ' ' << buf.st_size << ' ' << buf.st_mtime << endl;
            ifstream fi( di->string().c_str() );
            string tmp;
            // skip three lnes
            getline( fi, tmp );
            string MailFrom = "$4";
            string RcptTo = "$5";
            smatch m;
            if ( regex_search(tmp, m, REmf) ) {
              if ( m[1].matched ) {
                MailFrom = tmp.substr( m[1].first - tmp.begin(), m[1].second - m[1].first );
              }
            }
            getline( fi, tmp );
            if ( regex_search(tmp, m, RErt) ) {
              if ( m[1].matched ) {
                RcptTo = tmp.substr( m[1].first - tmp.begin(), m[1].second - m[1].first );
              }
            }
            rec r;
            r.name = di->leaf();
            r.time = buf.st_mtime;
            r.size = buf.st_size;
            r.MailFrom = MailFrom;
            r.RcptTo = RcptTo;
            v.push_back( r );
#if 0
            cout << "# ----- begin\n"
                 << "# delay \n" // << (time_prev == 0? 0 : ())
                 << "connect $1 $2\n"
                 << "expected 220\n"
                 << "EHLO $3\n"
                 << "expected 250\n"
                 << "MAIL FROM:<" << MailFrom << ">\n"
                 << "expected 250\n"
                 << "RCPT TO:<" << RcptTo << ">\n"
                 << "expected 250\n"
                 << "send $6 $7\n"
                 << "DATA\n"
                 << "expected 354"
                 << "include /opt/mcollection2/x2/" << di->leaf() << "\n"
                 << "expected 250\n"
                 << "wait\n"
                 << "QUIT\n"
                 << "expected 221\n"
                 << "disconnect\n"
                 << "# ----- end\n";
#endif
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

  // sort by time
  sort( v.begin(), v.end(), less_tm );

  // count samples
  generate( v.begin(), v.end(), incr() );

  for_each( v.begin(), v.end(), out );

  return 0;
}

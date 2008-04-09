// -*- C++ -*- Time-stamp: <04/06/16 12:51:35 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: smtp-batch.cc,v 1.1 2004/06/16 14:24:07 ptr Exp $"
#  else
#ident "@(#)$Id: smtp-batch.cc,v 1.1 2004/06/16 14:24:07 ptr Exp $"
#  endif
#endif


#include <mt/xmt.h>
#include <mt/time.h>
#include <sockios/sockstream>
#include <misc/args.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>

#include <boost/regex.hpp>

using namespace std;
using namespace boost;

struct conn
{
    conn()
      {
        tm[0].tv_sec  = tm[1].tv_sec  = tm[2].tv_sec  = 0;
        tm[0].tv_nsec = tm[1].tv_nsec = tm[2].tv_nsec = 0;
      }

    conn( const conn& _c )
      { tm[0] = _c.tm[0]; tm[1] = _c.tm[1]; tm[2] = _c.tm[2]; }

    timespec tm[3]; // time of send, time of response, elapsed interval (from tecol server)
};

typedef std::pair<std::string,conn> map_type;
typedef std::map<std::string,conn> container_type;
static container_type m;
static container_type finished;

enum command_type {
  UNKNOWN = 0,
  HELO,
  EHLO,
  VRFY,
  EXPN,
  NOOP,
  RSET,
  QUIT,
  DATA,
  HELP,
  MAIL,
  RCPT,
  _connect_,
  _disconnect_,
  _wait_,
  _include_,
  _send_,
  _expected_,
  _delay_,
  LAST_CMD
};

map<command_type,string> CMD;
sockstream tecol;
__impl::Condition tecol_ready;

void init_map()
{
  CMD[UNKNOWN] = "";
  CMD[HELO] = "HELO";
  CMD[EHLO] = "EHLO";
  CMD[VRFY] = "VRFY";
  CMD[EXPN] = "EXPN";
  CMD[NOOP] = "NOOP";
  CMD[RSET] = "RSET";
  CMD[QUIT] = "QUIT";
  CMD[DATA] = "DATA";
  CMD[HELP] = "HELP";
  CMD[MAIL] = "MAIL";
  CMD[RCPT] = "RCPT";
  CMD[_connect_] = "connect";
  CMD[_disconnect_] = "disconnect";
  CMD[_wait_] = "wait";
  CMD[_include_] = "include";
  CMD[_send_] = "send";
  CMD[_expected_] = "expected";
  CMD[_delay_] = "delay";
}

class RAWclient
{
  public:
    RAWclient()
      { }

    ~RAWclient()
      {
        s.close();
        sess << "== " << calendar_time( time(0) ) << endl;
      }

    void open( const string& host, int port, const string& log )
      {
        if ( !sess.is_open() ) {
          sess.open( log.c_str() );
          sess << "== " << calendar_time( time(0) ) << ' '
               << "this prg name and args" << endl;
        }
        s.open( host.c_str(), port );
        // _hostname = host;
        // _port = port;
        sess << "=+ " << calendar_time( time(0) ) 
             << ' ' << host << ':' << port << endl;
      }

    void close()
      {
        s.close();
        sess << "=- " << calendar_time( time(0) ) << endl;
        // sess.close();
      }

    // bool is_open() const
    //  { return s.is_open(); }

    // const string& hostname() const
    //  { return _hostname; }

    // int port() const
    //  { return _port; }

    void command( const string& str )
      {
        s << str << "\r\n";
        s.flush();
        sess << "<- " << str << "\r\n";
      }

    int response( string& text )
      {
        string line;
        text.clear();
        getline( s, line );
        sess << "-> " << line;
        while ( line.length() > 3 && line[3] == '-' ) {
          text += line.substr( 4 );
          getline( s, line );
          sess << "\n-> " << line;
        }
        text += line;
        stringstream str( line );
        int ret;
        str >> ret;
        getline( str, text );
        sess << endl;
        return ret;
      }

    string rawline()
      {
        string line;
        getline( s, line );
        sess << "-> " << line << endl;

        return line;
      }

    void write_silent( const string& str )
      {
        s << str;
        s.flush();
      }

    template <class InputIter>
    void write_silent( InputIter b, InputIter e )
      {
        ostream_iterator<char> o( s );
        copy( b, e, o );
        s.flush();
      }

    void write( const string& str )
      {
        s << str;
        /*
          if ( str is "^\r\n" ) {
            sess << "<- \n";
            skip NL
          }
         */
        sess << "<- " << str;
        s.flush();
      }

    void mark_in_log( const string& str )
      {
        sess << "!! " << str;
      }

    void import_in_log( const string& str )
      {
        sess << "<= " << str << '\n';
      }

  private:
    sockstream s;
    ofstream sess;

    // string _hostname;
    // int _port;
};

command_type cmd_token( const string& cmd )
{
  if ( cmd == "HELO" ) {
    return HELO;
  } else if ( cmd == "EHLO" ) {
    return EHLO;
  } else if ( cmd == "VRFY" ) {
    return VRFY;
  } else if ( cmd == "EXPN" ) {
    return EXPN;
  } else if ( cmd == "NOOP" ) {
    return NOOP;
  } else if ( cmd == "RSET" ) {
    return RSET;
  } else if ( cmd == "QUIT" ) {
    return QUIT;
  } else if ( cmd == "DATA" ) {
    return DATA;
  } else if ( cmd == "HELP" ) {
    return HELP;
  } else if ( cmd == "MAIL" ) {
    return MAIL;
  } else if ( cmd == "RCPT" ) {
    return RCPT;
  } else if ( cmd == "connect" ) {
    return _connect_;
  } else if ( cmd == "disconnect" ) {
    return _disconnect_;
  } else if ( cmd == "wait" ) {
    return _wait_;
  } else if ( cmd == "include" ) {
    return _include_;
  } else if ( cmd == "send" ) {
    return _send_;
  } else if ( cmd == "expected" ) {
    return _expected_;
  } else if ( cmd == "delay" ) {
    return _delay_;
  }

  return UNKNOWN;
}

void interpret( istream& is, const Argv& arg )
{
  string s;
  ostringstream re; // ( "(\'.*?\')|(\".*?\")" );
  // re.seekp( 5, ios_base::beg );
  // cout << re.tellp() << endl;
  re << "(\'.*?\')|(\".*?\")";
  for ( int j = 0; j < arg.size(); ++j ) {
    re << "|(\\$" << (j+1) << ")";
  }
  // cout << re.str().size() << endl;
  // cout << re.str() << endl;
  regex REcomment( "(\'.*?\')|(\".*?\")|(#.*)" );
  regex REqoutes( re.str() );
  // regex REname( "([[:space:]]+)((\'(.*?)\')|(\"(.*?)\")|(([[:alnum:]]|(\\\\.))+))" );
  regex REname( "(\'(.*?)\')|(\"(.*?)\")|(([[:word:]\\\\.\\$/,%~+-=])+)" );
  regex REescape( "(\\\\(.))" );

  re.str( "" );
  re.clear();
  re << "(?1$&)(?2$&)";

  for ( int j = 0; j < arg.size(); ++j ) {
    // "()" used here to separate ?N from replacement string,
    // that may contain digits.
    re << "(?" << (j+3) << "()" << arg[j] << ")";
  }
  // cout << re.str().size() << endl;
  // cout << re.str() << endl;
  string fmt = re.str();

  bool verbatim_mode = false;
  string verbatim_code;
  int ln = 0;

  RAWclient client;
  bool send_flag = false;
  string id;

  init_map();

  command_type last_command = UNKNOWN;

  char buf[64];
  gethostname( buf, 64 );
  string my_host_name = buf;

  string lfname;
  arg.assign( "-l", lfname );

  bool nodelay;
  arg.assign( "-nodelay", nodelay );

  while ( is.good() ) {
    getline( is, s );
    if ( !is.fail() ) {
      ++ln;
      s = regex_replace( s, REqoutes, fmt, boost::match_default | boost::format_all );
      cout << ln << ": " << s << endl;
      if ( !verbatim_mode ) {
        // strip comments
        string sr = regex_replace( s, REcomment, "(?1$&)(?2$&)(?3)", boost::match_default | boost::format_all );
        string::size_type p;
        if ( (p = sr.find( ">>>" )) != string::npos ) { // start verbatim
          verbatim_mode = true;
          verbatim_code = sr.substr( p + 3 );
          if ( last_command == DATA && send_flag ) {
            string tmp( "X-smtpgw-test: " );
            tmp += id;
            tmp += " action=recv\n";
            client.write( tmp );
            send_flag = false;
          }
          continue;
        }
        istringstream ss( sr );
        string cmd;
        ss >> cmd;
        if ( cmd.size() > 0 ) {
          command_type command = cmd_token( cmd );
          string host;
          int port;
          int expected;
          string rest;
          switch ( command ) {
            case _connect_:
              ss >> host >> port;
              // cout << "connect " << host << ":" << port << endl;
              client.open( host, port, lfname );
              break;
            case HELO:
            case EHLO:
            case VRFY:
            case EXPN:
              getline( ss, rest );
              client.command( CMD[command] + rest );
              last_command = command;
              // cout << CMD[command] << rest << endl;
              break;
            case NOOP:
            case RSET:
            case QUIT:
            case DATA:
              client.command( CMD[command] );
              // cout << CMD[command] << endl;
              last_command = command;
              break;
            case HELP:
              getline( ss, rest );
              client.command( CMD[command] + rest );
              // cout << CMD[command] << rest << endl;
              break;
            case MAIL:
            case RCPT:
              getline( ss, rest );
              client.command( CMD[command] + rest );
              last_command = command;
              // cout << CMD[command] << rest << endl;
              break;
            case _disconnect_:
              client.close();
              // cout << CMD[command] << endl;
              break;
            case _expected_:
              ss >> expected;
              getline( ss, rest );
              {
                string txt;
                int resp = client.response( txt );
                if ( resp != expected ) {
                  stringstream msg;
                  msg << "Unexpected return code after command "
                      << CMD[last_command] << ": "
                      << resp << " (expected " << expected << ")" << endl;
                  client.mark_in_log( msg.str() );
                  cerr << msg.str();
                }
                // cout << CMD[command] << rest << endl;
              }
              break;
            case _send_:
              ss >> host >> port;
              // cout << "send " << host << ":" << port << endl;
              send_flag = true;
              {
                stringstream ss;
                timespec t;
                __impl::Thread::gettime( &t );
                ss << t.tv_sec << "."
                   << setiosflags(ios_base::right) << setfill('0') << setw(9)
                   << t.tv_nsec << "-" << my_host_name;
                if ( !tecol.is_open() ) {
                  tecol.open( host.c_str(), port );
                  tecol_ready.set( true );
                }//  else if ( host changed or port changed ) {
                // }
                tecol << "id=" << ss.str() << " " << "action=out" << endl;
                id = ss.str();
              }
              break;
            case _wait_:
              // getline( tecol, rest );
              break;
            case _include_:
              {
                smatch m;
                getline( ss, rest );
                if ( regex_search( rest, m, REname ) ) {
                  if ( m[2].matched ) {
                    rest = rest.substr( m[2].first - rest.begin(), m[2].second - m[2].first );
                  }
                  if ( m[4].matched ) {
                    rest = rest.substr( m[4].first - rest.begin(), m[4].second - m[4].first );
                  }
                  if ( m[5].matched ) {
                    rest = rest.substr( m[5].first - rest.begin(), m[5].second - m[5].first );
                    // remove escape sign:
                    rest = regex_replace( rest, REescape, "(?1$2)", boost::match_default | boost::format_all );
                  }
                  if ( last_command == DATA && send_flag ) {
                    string tmp( "X-smtpgw-test: id=" );
                    tmp += id;
                    tmp += " action=recv\n";
                    client.write( tmp );
                    send_flag = false;
                  }
                  client.import_in_log( rest );
                  ifstream f( rest.c_str() );
                  f.unsetf( ios_base::skipws );
                  client.write_silent( istream_iterator<char>(f), istream_iterator<char>() );

                  if ( last_command == DATA ) {
                    client.write_silent( "\r\n.\r\n" );
                    last_command = UNKNOWN;
                  }
                }
              }
              break;
            case _delay_:
              getline( ss, rest );
              if ( !nodelay ) {
                istringstream str( rest );
                timespec t;
                str >> t.tv_sec;
                if ( !str.fail() ) {
                  if ( t.tv_sec > 0 ) {
                    t.tv_nsec = lrand48() % 2000000000;
                    if ( t.tv_nsec > 1000000000 ) {
                      --t.tv_sec;
                      t.tv_nsec -= 1000000000;
                    }
                  } else {
                    t.tv_nsec = lrand48() % 1000000000;
                  }
                  __impl::Thread::delay( &t );
                }
              }
              break;
            default:
              cout << "** " << sr << endl;
              break;
          }
        }
      } else { // verbatim mode
        string::size_type p;
        string eov( "<<<" );
        eov += verbatim_code;
        if ( (p = s.find( eov )) != string::npos ) { // end of verbatim
          verbatim_mode = false;
          verbatim_code = "";
          if ( last_command == DATA ) {
            client.write( "\r\n.\r\n" );
            last_command = UNKNOWN;
          }
        } else {
          client.write( s );
        }
      }        
    }
  }
}

int read_tecol( void * )
{
  tecol_ready.try_wait();

  string s;

  while ( tecol.good() ) {
    getline( tecol, s );
    if ( !tecol.fail() ) {
      cout << s << endl;
    }
  }

  return 0;
}

int main( int argc, char * const *argv )
{
  // int port;
  // string host;

  try {
    Argv arg;
    arg.copyright( "Copyright (C) K     sky Lab, 2003, 2004" );
    arg.brief( "Mail script interpreter" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-f", string( "" ), "script file, default stdin" );
    arg.option( "-l", string( "session.log" ), "log file, default session.log" );
    arg.option( "-nodelay", false, "ignore 'delay' directive in script, default is false (i.e. not ignore)" );
    try {
      arg.parse( argc, argv );
    }
    catch ( std::invalid_argument& err ) {
      cerr << err.what() << endl;
      arg.print_help( cerr );
      throw 1;
    }
    bool turn;
    if ( arg.assign( "-h", turn ) ) {
      arg.print_help( cout );
      throw 0;
    }

    string fname;
    arg.assign( "-f", fname );

    tecol_ready.set( false );

    __impl::Thread t( read_tecol, 0, 0, __impl::Thread::detached );

    if ( fname.length() != 0 ) {
      ifstream script( fname.c_str() );
      interpret( script, arg );
    } else {
      interpret( cin, arg );
    }
  }
  catch ( runtime_error& err ) {
    cerr << err.what() << endl;
    return -1;
  }
  catch ( std::exception& err ) {
    cerr << err.what() << endl;
    return -1;
  }
  catch ( int r ) {
    return r;
  }

  return 0;
}

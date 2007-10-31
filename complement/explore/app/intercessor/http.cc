// -*- C++ -*- Time-stamp: <07/03/07 15:17:27 ptr>

#include <net/http.h>
#include <istream>
#include <ostream>
#include <iomanip>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <iterator>

namespace http {

using namespace std;

boost::regex cmd_re( "^((?:OPTIONS)|(?:GET)|(?:HEAD)|(?:POST)|(?:PUT)|(?:DELETE)|(?:TRACE)|(?:CONNECT))(?:\\s+)(\\S+)(?:\\s+)HTTP/1\\.(0|1)\\r?$" );
boost::regex header_re( "^([[:word:]-]+):\\s+(\\S*(?:\\s+\\S+)*)\\s*\\r?$" );
boost::regex response_re( "^HTTP/1\\.(0|1)\\s+(\\d{3})\\s+(\\S*(?:\\s+\\S+)*)\\s*\\r?$" );

const string opt   = "OPTIONS";
const string get   = "GET";
const string head  = "HEAD";
const string post  = "POST";
const string put   = "PUT";
const string del   = "DELETE";
const string trace = "TRACE";
const string conn  = "CONNECT";
const string http10 = "HTTP/1.0";
const string http11 = "HTTP/1.1";


std::ostream& operator <<( std::ostream& s, const command& c )
{
  if ( c._cmd == command::UNKNOWN || c._proto == command::UNSPEC ) {
    s.setstate( ios_base::failbit );
    return s;
  }

  switch ( c._cmd ) {
    case command::OPTIONS:
      s << opt;
      break;
    case command::GET:
      s << get;
      break;
    case command::HEAD:
      s << head;
      break;
    case command::POST:
      s << post;
      break;
    case command::PUT:
      s << put;
      break;
    case command::DELETE:
      s << del;
      break;
    case command::TRACE:
      s << trace;
      break;
    case command::CONNECT:
      s << conn;
      break;
    default:
      s.setstate( ios_base::failbit );
      break;
  }
  return s << ' ' << c._url << ' ' << (c._proto == command::HTTP10 ? http10 : http11) << "\r\n";
}

std::istream& operator >>( std::istream& s, command& c )
{
  string line;
  boost::smatch ma;

  getline( s, line );

  if ( !s.fail() && regex_match( line, ma, cmd_re /* , boost::regex_constants::match_partial */ ) ) {
    if ( ma[1] == opt ) {
      c._cmd = command::OPTIONS;
    } else if ( ma[1] == get ) {
      c._cmd = command::GET;
    } else if ( ma[1] == head ) {
      c._cmd = command::HEAD;
    } else if ( ma[1] == post ) {
      c._cmd = command::POST;
    } else if ( ma[1] == put ) {
      c._cmd = command::PUT;
    } else if ( ma[1] == del ) {
      c._cmd = command::DELETE;
    } else if ( ma[1] == trace ) {
      c._cmd = command::TRACE;
    } else if ( ma[1] == conn ) {
      c._cmd = command::CONNECT;
    } else {
      s.setstate( ios_base::failbit );
      return s;
    }

    c._url = ma[2];

    if ( ma[3] == '0' ) {
      c._proto = command::HTTP10;
    } else if ( ma[3] == '1' ) {
      c._proto = command::HTTP11;
    } else {
      s.setstate( ios_base::failbit );
    }
  } else {
    s.setstate( ios_base::failbit );
  }
  return s;
}

std::ostream& operator <<( std::ostream& s, const header& h )
{
  if ( h._val.first.length() == 0 ) {
    return s;
  }
  return s << h._val.first << ": " << h._val.second << "\r\n";
}

std::istream& operator >>( std::istream& s, header& h )
{
  string line;
  boost::smatch ma;

  getline( s, line );
  if ( !s.fail() && regex_match( line, ma, header_re /* , boost::regex_constants::match_partial */ ) ) {
    h._val.first  = ma[1];
    h._val.second = ma[2];
  } else {
    s.setstate( ios_base::failbit );
  }
  return s;
}

boost::regex cookie_re( "(?:(\\w+)=([^;]*)(?:;\\s+)?)*" );

cookie::cookie( const header& h )
{
  if ( h.key() == "Set-Cookie" ) {
    boost::smatch ma;
    if ( regex_search( h.value(), ma, cookie_re, boost::match_extra ) ) {
#ifdef BOOST_REGEX_MATCH_EXTRA
      for ( int j = 0; j < ma.captures(1).size(); ++j ) {
        if ( ma.captures(1)[j] == "path" ) {
          _path = ma.captures(2)[j];
        } else if ( ma.captures(1)[j] == "expires" ) {
          _expires = ma.captures(2)[j];
        } else if ( ma.captures(1)[j] == "Expires" ) {
          _expires = ma.captures(2)[j];
        } else if ( ma.captures(1)[j] == "domain" ) {
          _domain = ma.captures(2)[j];
        } else if ( ma.captures(1)[j] == "comment" ) {
        } else if ( ma.captures(1)[j] == "max-age" ) {
        } else if ( ma.captures(1)[j] == "version" ) {
        } else {
          _val.first  = ma.captures(1)[j];
          _val.second = ma.captures(2)[j];
        }
      }
#endif
    }
  }
}

std::ostream& operator <<( std::ostream& s, const base_response& r )
{
  s << (r._proto == command::HTTP10 ? http10 : http11) << ' ' << setfill( '0' ) << setw(3) << r._code << ' ';
  switch ( r._code ) {
    case 100:
      s << "Continue";
      break;
    case 101:
      s << "Switching Protocols";
      break;
    case 200:
      s << "Ok";
      break;
    case 201:
      s << "Created";
      break;
    case 202:
      s << "Accepted";
      break;
    case 203:
      s << "Non-Authoritative Information";
      break;
    case 204:
      s << "No Content";
      break;
    case 205:
      s << "Reset Content";
      break;
    case 206:
      s << "Partial Content";
      break;
    case 300:
      s << "Multiple Choices";
      break;
    case 301:
      s << "Moved Permanently";
      break;
    case 302:
      s << "Found";
      break;
    case 303:
      s << "See Other";
      break;
    case 304:
      s << "Not Modified";
      break;
    case 305:
      s << "Use Proxy";
      break;
    case 306:
      s << "(Unused)";
      break;
    case 307:
      s << "Temporary Redirect";
      break;
    case 400:
      s << "Bad Request";
      break;
    case 401:
      s << "Unauthorized";
      break;
    case 402:
      s << "Payment Required";
      break;
    case 403:
      s << "Forbidden";
      break;
    case 404:
      s << "Not Found";
      break;
    case 405:
      s << "Method Not Allowed";
      break;
    case 406:
      s << "Not Acceptable";
      break;
    case 407:
      s << "Proxy Authentication Required";
      break;
    case 408:
      s << "Request Timeout";
      break;
    case 409:
      s << "Conflict";
      break;
    case 410:
      s << "Gone";
      break;
    case 411:
      s << "Length Required";
      break;
    case 412:
      s << "Precondition Failed";
      break;
    case 413:
      s << "Request Entity Too Large";
      break;
    case 414:
      s << "Request-URI Too Long";
      break;
    case 415:
      s << "Unsupported Media Type";
      break;
    case 416:
      s << "Requested Range Not Satisfiable";
      break;
    case 417:
      s << "Expectation Failed";
      break;
    case 500:
      s << "Internal Server Error";
      break;
    case 501:
      s << "Not Implemented";
      break;
    case 502:
      s << "Bad Gateway";
      break;
    case 503:
      s << "Service Unavailable";
      break;
    case 504:
      s << "Gateway Timeout";
      break;
    case 505:
      s << "HTTP Version Not Supported";
      break;
    default:
      s.setstate( ios_base::failbit );
      break;
  }
  return s << "\r\n";
}

std::istream& operator >>( std::istream& s, base_response& r )
{
  try {
    string line;
    boost::smatch ma;

    getline( s, line );
    if ( !s.fail() && regex_match( line, ma, response_re /* , boost::regex_constants::match_partial */ ) ) {
      if ( ma[1] == '0' ) {
        r._proto = command::HTTP10;
      } else if ( ma[1] == '1' ) {
        r._proto = command::HTTP11;
      } else {
        s.setstate( ios_base::failbit );
        return s;
      }
      r._code = boost::lexical_cast<int>(ma[2]);
    } else {
      s.setstate( ios_base::failbit );
    }
  }
  catch ( boost::bad_lexical_cast& err ) {
    s.setstate( ios_base::failbit );
  }
  return s;
}

} // namespace http

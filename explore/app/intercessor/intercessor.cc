// -*- C++ -*- Time-stamp: <07/03/07 15:53:23 ptr>

#include "intercessor.h"

#include <iostream>
#include <mt/xmt.h>
#include <sockios/sockstream>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <misc/tfstream>
#include <mt/lfstream.h>
#include <algorithm>
#ifndef STLPORT
#include <ext/algorithm>
using namespace __gnu_cxx;
#endif
#include <iterator>
#include <unistd.h>

extern boost::filesystem::path dir;

namespace intr {

using namespace std;
using namespace xmt;

void httprq::pack( std::ostream& s ) const
{
  s << http::body( true ) << rq;
}

void httprq::net_pack( std::ostream& s ) const
{
  s << http::body( true ) << rq;
}

void httprq::unpack( std::istream& s )
{
  s >> http::body( true ) >> rq;
}

void httprq::net_unpack( std::istream& s )
{
  s >> http::body( true ) >> rq;
}

void httprs::pack( std::ostream& s ) const
{
  s << http::body( true ) << rs;
}

void httprs::net_pack( std::ostream& s ) const
{
  s << http::body( true ) << rs;
}

void httprs::unpack( std::istream& s )
{
  s >> http::body( true ) >> rs;
}

void httprs::net_unpack( std::istream& s )
{
  s >> http::body( true ) >> rs;
}

void Intercessor::request( const stem::Event_base<intr::httprq>& ev )
{
  namespace fs = boost::filesystem;

  http::request rq = ev.value().rq;

  http::request::headers_container_type::const_iterator f = rq.search( "X-API-ReportFileName" );
  http::request::headers_container_type::const_iterator h = rq.search( "Host" );

  if ( h == rq.headers().end() || f == rq.headers().end() ) {
    stem::Event r_ev( RESPONCE_NOK );
    r_ev.dest( ev.src() );

    Send( r_ev );

    return;
  }

  string tnm = dir.string() + "/" + f->value();
  string tnmh =  dir.string() + "/" + f->value() + ".head";

  fs::path p1;
  fs::path p2;

  fs::path p01 = fs::system_complete( fs::path( tnm, fs::native ) );
  fs::path p02 = fs::system_complete( fs::path( tnmh, fs::native ) );

  try {
    string hostname = h->value();
    string::size_type pos = hostname.find( ':' );
    int port = 80;
    if ( pos != string::npos ) {
      port = boost::lexical_cast<int>( hostname.substr( pos + 1 ) );
      hostname.erase( pos );
    }

    misc::tfstream of( tnm.c_str() );
    misc::tfstream ofh( tnmh.c_str() );

    p1 = fs::system_complete( fs::path( of.name(), fs::native ) );
    p2 = fs::system_complete( fs::path( ofh.name(), fs::native ) );

    if ( !of.good() ) {
      throw runtime_error( string( "can't write to file '" ) + of.name() + "'" );
    }

    if ( !ofh.good() ) {
      throw runtime_error( string( "can't write to file '" ) + ofh.name() + "'" );
    }

    http::request::headers_container_type::iterator xi = find( rq.headers().begin(), rq.headers().end(), string( "X-API-ReportFileName" ) );
    rq.headers().erase( xi );

    xi = find( rq.headers().begin(), rq.headers().end(), string( "Proxy-Connection" ) );
    if ( xi != rq.headers().end() ) {
      rq.headers().erase( xi );
    }

    string url( rq.head().URL() );
    pos = url.find( "http://" );
    if ( pos != 0 ) {
      throw runtime_error( string( "invalid or unsupported URL: '" + url + "'" ) );
    }

    url.erase( 0, 7 ); // http://

    // host:port
    pos = url.find( "/" ); 
    if ( pos == string::npos ) {
      url = "/";
    } else {
      url.erase( 0, pos );
    }

    rq.head().URL( url );

    // connect to real server
    sockstream realsrv( hostname.c_str(), port );

    if ( !realsrv.good() ) {
      throw runtime_error( "connection failed to " + hostname + ":" + boost::lexical_cast<string>(port) );
    }

    // write request to real server
    (realsrv << noskipws << http::body( true ) << rq).flush();

    http::response rs;

    // read response from real server
    // don't read body to http::response, I will copy it directly from tcp stream to file
    realsrv >> noskipws >> http::body( false ) >> rs;

    if ( rs.head().code() < 400 ) {
      stem::EventVoid r_ev( RESPONCE_OK );
      r_ev.dest( ev.src() );

      Send( r_ev );
    } else {
      http::response::headers_container_type::iterator cl = rs.search( "Content-Length" );
      stringstream out;

      if ( cl == rs.headers().end() ) {
        cl = rs.search( "Transfer-Encoding" );
        if ( cl != rs.headers().end() && cl->value() == "chunked" ) {
          int chunk_size = -1;
          int count = 0;
          realsrv >> std::hex >> chunk_size >> std::dec;
          istreambuf_iterator<char> istr( realsrv.rdbuf() );
          ++istr; ++istr; // oh, check CR/LF
          // of.lock_ex();
          ostreambuf_iterator<char> ostr( out.rdbuf() );
          while ( chunk_size > 0 && realsrv.good() && out.good() ) {
            count += chunk_size;
            while ( chunk_size-- > 0 && realsrv.good() && out.good() ) {
              *ostr++ = *istr++;
            }
            ++istr; ++istr;
            realsrv >> std::hex >> chunk_size >> std::dec;
            ++istr; ++istr; // oh, check CR/LF
          }
          rs.headers().erase( cl );
          rs.headers().push_back( http::header( "Content-Length", boost::lexical_cast<std::string>( count ) ) );
        } else {
          throw runtime_error( "no content returned" );
        }
      } else {
        int len = boost::lexical_cast<int>( cl->value() );
        if ( len <= 0 ) {
          throw runtime_error( "zero length content returned" );
        }
        // and write all body to another file
        istreambuf_iterator<char> istr( realsrv.rdbuf() );
        // of.lock_ex();
        ostreambuf_iterator<char> ostr( out.rdbuf() );
        while ( len-- > 0 && realsrv.good() && out.good() ) {
          *ostr = *istr;
          if ( len > 0 ) {
            ++istr;
            ++ostr;
          }
        }
      }
      rs.body() = out.str();
      rs.bodyf( true );

      throw rs;
    }

    // check Content-Length header (hmm, I can't process something else)
    http::response::headers_container_type::iterator cl = rs.search( "Content-Length" );

    if ( cl == rs.headers().end() ) {
      cl = rs.search( "Transfer-Encoding" );
      if ( cl != rs.headers().end() && cl->value() == "chunked" ) {
        int chunk_size = -1;
        int count = 0;
	realsrv >> std::hex >> chunk_size >> std::dec;
        istreambuf_iterator<char> istr( realsrv.rdbuf() );
        ++istr; ++istr; // oh, check CR/LF
        // of.lock_ex();
        ostreambuf_iterator<char> ostr( of.rdbuf() );
        while ( chunk_size > 0 && realsrv.good() && of.good() ) {
          count += chunk_size;
          while ( chunk_size-- > 0 && realsrv.good() && of.good() ) {
            *ostr++ = *istr++;
          }
          ++istr; ++istr;
          realsrv >> std::hex >> chunk_size >> std::dec;
          ++istr; ++istr; // oh, check CR/LF
        }
        of.flush();
        rs.headers().erase( cl );
        rs.headers().push_back( http::header( "Content-Length", boost::lexical_cast<std::string>( count ) ) );
        // of.unlock();
      } else {
        throw runtime_error( "no content returned" );
      }
    } else {
      int len = boost::lexical_cast<int>( cl->value() );
      if ( len <= 0 ) {
	throw runtime_error( "zero length content returned" );
      }
      // and write all body to another file
      istreambuf_iterator<char> istr( realsrv.rdbuf() );
      // of.lock_ex();
      ostreambuf_iterator<char> ostr( of.rdbuf() );
      while ( len-- > 0 && realsrv.good() && of.good() ) {
        *ostr = *istr;
        if ( len > 0 ) {
          ++istr;
          ++ostr;
        }
      }
      of.flush();
      // of.unlock();
    }

    // write headers (from response) to one file
    // ofh.lock_ex();
    ofh << rs;
    ofh.flush();
    // ofh.unlock();
  }
  catch ( boost::bad_lexical_cast& err ) {
    cerr << err.what() << endl;

    stem::Event r_ev( RESPONCE_NOK );
    r_ev.dest( ev.src() );

    Send( r_ev );
  }
  catch ( http::response& rs ) {
    stem::Event_base<intr::httprs> r_ev( RESPONCE_UNP );
    r_ev.dest( ev.src() );
    r_ev.value().rs = rs;
    Send( r_ev );
    if ( fs::exists( p1 ) ) {
      fs::remove( p1 );
    }
    if ( fs::exists( p2 ) ) {
      fs::remove( p2 );
    }
  }
  catch ( runtime_error& err ) {
    stem::Event r_ev( RESPONCE_NOK );
    r_ev.dest( ev.src() );

    Send( r_ev );
    if ( fs::exists( p1 ) ) {
      fs::remove( p1 );
    }
    if ( fs::exists( p2 ) ) {
      fs::remove( p2 );
    }
    cerr << err.what() << endl;
  }
    
  if ( fs::exists( p1 ) ) {
    ::link( p1.string().c_str(), p01.string().c_str() );
    ::unlink( p1.string().c_str() );
  }
  if ( fs::exists( p2 ) ) {
    ::link( p2.string().c_str(), p02.string().c_str() );
    ::unlink( p2.string().c_str() );
  }
}

DEFINE_RESPONSE_TABLE( Intercessor )
  EV_Event_base_T_( ST_NULL, INTR_RQ, request, intr::httprq )
END_RESPONSE_TABLE

} // namespace intr

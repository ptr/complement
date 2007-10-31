// -*- C++ -*- Time-stamp: <07/03/07 16:11:19 ptr>

#ifndef __http_h
#define __http_h

#include <istream>
#include <ostream>
#include <string>
#include <sstream>
#include <functional>
#include <iterator>
#include <list>
#include <boost/lexical_cast.hpp>

namespace http {

class command
{
  public:

    enum cmd {
      UNKNOWN,
      GET,
      PUT,
      HEAD,
      POST,
      OPTIONS,
      DELETE,
      TRACE,
      CONNECT
    };

    enum proto {
      UNSPEC,
      HTTP10,
      HTTP11
    };

    command() :
        _cmd( UNKNOWN ),
        _proto( UNSPEC )
      { }

    command( const command& c ) :
        _cmd( c._cmd ),
        _proto( c._proto ),
        _url( c._url )
      { }

    cmd value() const
      { return _cmd; }

    proto protocol() const
      { return _proto; }

    const std::string& URL() const
      { return _url; }

    void value( cmd c )
      {  _cmd = c; }

    void protocol( proto p )
      { _proto = p; }

    void URL( const std::string& url )
      { _url = url; }

  private:
    cmd   _cmd;
    proto _proto;
    std::string _url;

    friend std::ostream& operator <<( std::ostream& s, const command& c );
    friend std::istream& operator >>( std::istream& s, command& c );
};

std::ostream& operator <<( std::ostream& s, const command& c );
std::istream& operator >>( std::istream& s, command& c );

class header
{
  public:
    header() :
        _val( std::string(), std::string() )
      { }

    header( const std::string& k, const std::string& v ) :
        _val( k, v )
      { }

#if !defined(__GNUC__) || (__GNUC__ > 3) || ((__GNUC__ == 3) && \
                                             ((__GNUC_MINOR__ > 4) || ((__GNUC_MINOR__ == 4) && (__GNUC_PATCHLEVEL__ > 4)) ) )
    template <class T>
    header( const std::string& k, const T& v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    template <class T>
    header( const char *k, const T& v ) :
        _val( std::string(k), boost::lexical_cast<std::string>(v) )
      { }
#else
    header( const std::string& k, int v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, unsigned v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, long v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, unsigned long v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, short v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, unsigned short v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, char v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, signed char v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, unsigned char v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, double v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, float v ) :
        _val( k, boost::lexical_cast<std::string>(v) )
      { }
    header( const std::string& k, const char *v ) :
        _val( k, std::string(v) )
      { }
    header( const char *k, const char *v ) :
        _val( std::string(k), std::string(v) )
      { }
#endif


    header( const header& h ) :
        _val( h._val )
      { }

    const std::string& key() const
      { return _val.first; }

    const std::string& value() const
      { return _val.second; }

    void key( const std::string& k )
      { _val.first = k; }

    // template <>
    void value( const std::string& v )
      { _val.second = v; }

    template <class T>
    void value( const T& v )
      { _val.second = boost::lexical_cast<std::string>(v); }

    bool operator ==( const header& h ) const
      { return _val.first == h._val.first; }

    bool operator ==( const std::string& s ) const
      { return _val.first == s; }

  private:
    std::pair<std::string,std::string> _val;

    friend std::ostream& operator <<( std::ostream& s, const header& h );
    friend std::istream& operator >>( std::istream& s, header& h );
};

std::ostream& operator <<( std::ostream& s, const header& h );
std::istream& operator >>( std::istream& s, header& h );

class cookie
{
  public:

    cookie()
      { }

    cookie( const header& );

    const std::string& key() const
      { return _val.first; }

    const std::string& value() const
      { return _val.second; }

    void key( const std::string& k )
      { _val.first = k; }

    // template <>
    void value( const std::string& v )
      { _val.second = v; }

  private:
    std::pair<std::string,std::string> _val;
    std::string _domain;
    std::string _path;
    std::string _expires;
};

class base_response
{
  public:
    base_response() :
        _code( 0 ),
        _proto( command::UNSPEC )
      { }

    base_response( int c, command::proto p = command::HTTP11 ) :
        _code( c ),
        _proto( p )
      { }

    base_response( const base_response& r ) :
        _code( r._code ),
        _proto( r._proto )
      { }

    int code() const
      { return _code; }

    command::proto protocol() const
      { return _proto; }

    void code( int c )
      { _code = c; }

    void protocol( command::proto p )
      { _proto = p; }

  private:
    int _code;
    command::proto _proto;

    friend std::ostream& operator <<( std::ostream& s, const base_response& r );
    friend std::istream& operator >>( std::istream& s, base_response& r );
};

std::ostream& operator <<( std::ostream& s, const base_response& r );
std::istream& operator >>( std::istream& s, base_response& r );

template <class R> class message_start;
template <class R> typename std::ostream& operator <<( typename std::ostream& s, const message_start<R>& r );
template <class R> typename std::istream& operator >>( typename std::istream& s, message_start<R>& r );



struct __imsg_proxy
{
    __imsg_proxy( bool v ) :
        _v( v )
      { }

    bool _v;
};

inline __imsg_proxy body( bool v )
{ return __imsg_proxy( v ); }

struct __imsg_proxy_istream
{
    __imsg_proxy_istream( std::istream& s, const __imsg_proxy& p ) :
        _s( s ),
        _v( p._v )
      { }

    std::istream& _s;
    bool _v;
};

struct __imsg_proxy_ostream
{
    __imsg_proxy_ostream( std::ostream& s, const __imsg_proxy& p ) :
        _s( s ),
        _v( p._v )
      { }

    std::ostream& _s;
    bool _v;
};

inline __imsg_proxy_istream operator >>( std::istream& s, const __imsg_proxy& p )
{ return __imsg_proxy_istream( s, p ); }

inline __imsg_proxy_ostream operator <<( std::ostream& s, const __imsg_proxy& p )
{ return __imsg_proxy_ostream( s, p ); }

template <class R>
class message_start
{
  public:
    typedef std::list<header> headers_container_type;

    message_start() :
        _bodyf( false )
      { }

    message_start( const message_start& r ):
        _m( r._m ),
        _body( r._body ),
        _bodyf( r._bodyf )
      {
        std::copy( r._headers.begin(), r._headers.end(), std::back_insert_iterator<headers_container_type>(_headers) );
      }

    R& head()
      { return _m; }

    headers_container_type& headers()
      { return _headers; }

    const R& head() const
      { return _m; }

    const headers_container_type& headers() const
      { return _headers; }

    typename headers_container_type::iterator search( const typename std::string& s )
      { return std::find( _headers.begin(),  _headers.end(), s ); }

    typename headers_container_type::const_iterator search( const typename std::string& s ) const
      { return std::find( _headers.begin(),  _headers.end(), s ); }

    std::string& body()
      { return _body; }

    const std::string& body() const
      { return _body; }

    void bodyf( bool rb ) const
      { _bodyf = rb; }

  private:
    R _m;
    headers_container_type _headers;
    std::string _body;
    mutable bool _bodyf;

    friend typename std::ostream& operator << <R>( typename std::ostream& s, const message_start<R>& r );
    friend typename std::istream& operator >> <R>( typename std::istream& s, message_start<R>& r );
};

template <class R>
typename std::ostream& operator <<( typename std::ostream& s, const message_start<R>& r )
{
  if ( !s.good() ) {
    return s;
  }
  s << r._m;
  for ( typename message_start<R>::headers_container_type::const_iterator i =  r.headers().begin(); i != r.headers().end(); ++i ) {
    s << *i;
  }
  if ( r._bodyf && r.body().length() > 0 && r.search( "Content-Length" ) == r.headers().end() ) {
    s << typename http::header( "Content-Length", r.body().length() );
  }
  s << "\r\n";
  if ( r._bodyf && r.body().length() > 0 ) {
    s << r._body;
  }
  return s;
}

template <class R>
typename std::istream& operator >>( typename std::istream& s, message_start<R>& r )
{
  s >> r._m;

  std::string line;
  http::header h;

  while ( !getline( s, line ).fail() && line.length() > 0 && line != "\r" ) {
    // line += '\n';
    std::stringstream str( line );
    str >> h;
    r._headers.push_back( h );
  }

  if ( r._bodyf ) {
    // check Content-Length header (hmm, I can't process something else)
    typename message_start<R>::headers_container_type::iterator cl = r.search( "Content-Length" );

    if ( cl != r.headers().end() ) {
      int len = boost::lexical_cast<int>( cl->value() );

      if ( len > 0 ) {
        // and write all body to another file
        r._body.reserve( len );
        std::istreambuf_iterator<char> istr( s.rdbuf() );
        while ( len-- > 0 && s.good() ) {
          r._body += *istr;
          if ( len > 0 ) {
            ++istr;
          }
        }
      }
    } else {
      cl = r.search( "Transfer-Encoding" );
      if ( cl != r.headers().end() && cl->value() == "chunked" ) {
        int chunk_size = -1;
        int count = 0;
        bool skws = (s.flags() & std::ios_base::skipws) != 0;
	s >> std::noskipws >> std::hex >> chunk_size >> std::dec;
        std::istreambuf_iterator<char> istr( s.rdbuf() );
        ++istr; ++istr; // oh, check CR/LF
        count += chunk_size;
        while ( chunk_size > 0 && s.good() ) {
          r._body.reserve( chunk_size + r._body.length() );
          while ( chunk_size-- > 0 && s.good() ) {
            r._body += *istr++;
          }
          ++istr; ++istr;
          s >> std::hex >> chunk_size >> std::dec;
          ++istr; ++istr; // oh, check CR/LF
        }
        r._headers.erase( cl );
        r._headers.push_back( header( "Content-Length", boost::lexical_cast<std::string>(r._body.length()) ) );
        if ( skws ) {
          s.setf( std::ios_base::skipws );
        }
      } else { // server not HTTP 1.1-compliant (RFC2616, 4.4)
        copy( std::istreambuf_iterator<char>(s.rdbuf()), std::istreambuf_iterator<char>(), back_inserter(r._body) );
      }
    }
  }

  return s;
}

template <class R>
inline std::istream& operator >>( const __imsg_proxy_istream& _sp, message_start<R>& r )
{ 
  r.bodyf( _sp._v );
  return _sp._s >> r;
}

template <class R>
inline std::ostream& operator <<( const __imsg_proxy_ostream& _sp, const message_start<R>& r )
{ 
  r.bodyf( _sp._v );
  return _sp._s << r;
}

typedef message_start<command> request;
typedef message_start<base_response> response;

} // namespace http

#endif // __http_h

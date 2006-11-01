// -*- C++ -*- Time-stamp: <05/11/17 01:36:06 ptr>

#ifndef __resolver_res_ip_h
#define __resolver_res_ip_h

#if (defined(__FreeBSD__) || defined(__OpenBSD__))
#define res_search x_res_search
#endif

#include <string>
#include <queue>
#include <map>
#include <set>
#include <list>
#include <iterator>
#include <stdexcept>
#include <sstream>

#ifdef __linux
# include <stdint.h>
#else
# include <sys/types.h>
#endif

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include <iostream>

// #include <vector>

#include <boost/regex.hpp>

namespace resolver {

namespace details {

int x_ns_initparse(const u_char *msg, int msglen, ns_msg *handle);
int x_ns_parserr(ns_msg *handle, ns_sect section, int rrnum, ns_rr *rr);

class ns_answer
{
  private:
    union ns16 {
        unsigned char c[2];
        uint16_t      s;
    };

  public:
    ns_answer( unsigned char *b, int len );

    int size() const
      { return ns_msg_count( _answer, ns_s_an ); }

    class iterator :
        public std::forward_iterator<ns_answer,size_t>
    {
      public:

        iterator( ns_sect s = ns_s_an ) :
            msg( 0 ),
            n( -1 ),
            sect( s )
          {
          }

        const ns_rr& operator *() const
          {
            return _rr;
          }

        iterator& operator ++()
          {
            if ( x_ns_parserr( msg, ns_s_an, ++n, &_rr ) < 0 ) {
              n = -1;
            }
            return *this;
          }

        iterator& operator ++(int)
          {
            if ( x_ns_parserr( msg, ns_s_an, n++, &_rr ) < 0 ) {
              n = -1;
            }
            return *this;
          }

        bool operator ==(const iterator& i ) const
          {
            if ( (n == -1) && (i.n == -1) ) {
              return true;
            }
            if ( (n == i.n) && (msg == i.msg) && (sect == i.sect) ) {
              return true;
            }
            return false;
          }

        bool operator !=(const iterator& i ) const
          {
            if ( (n == -1) && (i.n == -1) ) {
              return false;
            }
            if ( (n != i.n) || (msg != i.msg) || (sect != i.sect) ) {
              return true;
            }
            return false;
          }

      private:
        ns_msg *msg;
        int n;
        ns_rr  _rr;
        ns_sect sect;

        friend class ns_answer;
    };

    iterator begin( ns_sect s = ns_s_an );
    iterator end( ns_sect s = ns_s_an )
      { return iterator(s); }

  private:

    ns_msg _answer;
    friend class ns_answer::iterator;
};

class priority_ip
{
  public:
    typedef std::pair<int,uint32_t> pip_t; // (score,ip)

  private:
    typedef std::priority_queue<pip_t> pip_container_type;

  public:
    typedef pip_container_type::value_type::second_type value_type;
    typedef pip_container_type::value_type::first_type penalty_type;

    priority_ip() :
        max_penalty( -1 )
      { }

    static const value_type bad_ip;

    bool empty() const
      { return ips.empty(); }

    pip_container_type::size_type size() const
      { return ips.size(); }

    const value_type& top() const
      { return ips.top().second; }

    void penalty();
    void push( penalty_type p, value_type v );

  private:
    penalty_type max_penalty;
    pip_container_type ips;
};

inline bool operator <( const priority_ip::pip_t& l, const priority_ip::pip_t& r )
{
  return l.first < r.first;
}

//typedef std::pair<int,std::string> ref_domain_type; // container of type resolve_type index, domain (host) name
//typedef std::pair<time_t,ref_domain_type> expire_type; // expire time, ref_domain

//typedef std::priority_queue<expire_type> expire_container_type;

std::string decode_string( const unsigned char *first, const unsigned char *last, const unsigned char *ref_buf );

} // namespace details


template <class T>
struct member_function
{
    typedef T   class_type;
    typedef T * pointer_class_type;
    typedef T&  reference_class_type;
    typedef const T * const_pointer_class_type;
    typedef const T&  const_reference_class_type;
    typedef void (T::*pmf_type)( uint32_t, int );
};

class domain_processing_error :
    public std::runtime_error
{
  public:
    explicit domain_processing_error( const std::string& s ) :
        runtime_error( s )
      { }
};

class ip_resolving_error :
    public std::runtime_error
{
  public:
    explicit ip_resolving_error( const std::string& s ) :
        runtime_error( s )
      { }
};


/*
 * Expected host name, return (via back insert iterator)
 * all IPs for specified host name; nothing will be added in case of
 * failure.
 */
template <class BackInsertIterator>
void ip_by_name( const std::string& name, BackInsertIterator bi )
{
#if 1
  // in_addr inet;
  int _errno = 0;

  hostent _host;
#  ifndef __hpux
  char tmpbuf[4096];
#  else // __hpux
  hostent_data tmpbuf;
#  endif // __hpux
#  ifdef __linux
  hostent *host = 0;
  gethostbyname_r( name.c_str(), &_host, tmpbuf, 4096, &host, &_errno );
#  elif defined(__hpux)
  _errno = gethostbyname_r( name.cstr(), &_host, &tmpbuf );
  hostent *host = &_host;
#  elif defined(__sun)
  hostent *host = gethostbyname_r( name.c_str(), &_host, tmpbuf, 4096, &_errno );
#  else // !__linux !__hpux !__sun
#    error "Check port of gethostbyname_r"
#  endif // __linux __hpux __sun

  if ( host != 0 && host->h_length == sizeof(in_addr) ) {
    for ( char **_inet = host->h_addr_list; *_inet != 0; ++_inet ) {
      *bi++ = ((in_addr *)*_inet)->s_addr;
    }
  }

#else // !1
  unsigned char bufa[4096];

  int len = res_search( name.c_str(), ns_c_in, ns_t_a, bufa, 4096 );

  std::cerr << name << " " << len << std::endl;
  if ( len <= 0 ) {
    return;
  }

#if 0
  ns_msg answer_a;
  details::x_ns_initparse( bufa, len, &answer_a );
  int nra = ns_msg_count( answer_a, ns_s_an );
  // cerr << nra << endl;

  ns_rr rra;
  for ( int j = 0; j < nra; ++j ) {
    details::x_ns_parserr( &answer_a, ns_s_an, j, &rra );
    // cerr << "Type: " << ns_rr_type(rra) << endl;
    // cerr << "TTL: " << ns_rr_ttl(rra) << endl;
    // cerr << "Class: " << ns_rr_class(rra) << endl;
    // cerr << "Name: " << ns_rr_name(rra) << endl; // \0?

    if ( ns_rr_rdlen(rra) == sizeof(in_addr) /* max(sizeof(in_addr), sizeof(in6_addr)) */ ) {
      in_addr *a = (in_addr *)ns_rr_rdata(rra);
      // cerr << hex << a->s_addr << dec << endl;

      // cerr << "Result: " << domain << " " << priority << " "
      //     << hex << a->s_addr << dec << endl;
      *bi++ = a->s_addr;
    }
  }
#else
  details::ns_answer answer( bufa, len );
  for ( typename details::ns_answer::iterator j = answer.begin(); j != answer.end(); ++j ) {
    if ( ns_rr_rdlen(*j) == sizeof(in_addr) /* max(sizeof(in_addr), sizeof(in6_addr)) */ ) {
      // in_addr *a = (in_addr *)ns_rr_rdata(*j);
      // cerr << hex << a->s_addr << dec << endl;

      // cerr << "Result: " << domain << " " << priority << " "
      //     << hex << a->s_addr << dec << endl;
      *bi++ = ((in_addr *)ns_rr_rdata(*j))->s_addr;
    }
  }
#endif // !0
#endif // 1
}

/*
 * Expected domain name, return (via back insert iterator)
 * all MX records for specified domain; nothing will be added in case of
 * failure.
 */
template <class BackInsertIterator>
void mx_for_domain( const std::string& name, BackInsertIterator bi )
{
  unsigned char bufa[4096];
  int len = res_search( name.c_str(), ns_c_in, ns_t_mx, bufa, 4096 );

  if ( len <= 0 ) {
    return;
  }

  details::ns_answer answer( bufa, len );
  for ( typename details::ns_answer::iterator j = answer.begin(); j != answer.end(); ++j ) {
    uint16_t priority = ntohs(*(uint16_t*)ns_rr_rdata(*j)); // not used yet...

    const unsigned char *rdata = ns_rr_rdata(*j);
    const unsigned char *edata = rdata + ns_rr_rdlen(*j);
    rdata += sizeof(uint16_t);

    std::string mx = details::decode_string( rdata, edata, bufa );

    // The name ordinary returned as name.domain.com. (dot at end).
    // Remove it.
    std::string::size_type pos = mx.rfind( '.' );

    if ( pos != std::string::npos ) {
      if ( pos == (mx.size() - 1) ) {
        mx.erase( pos, 1 );
      }
    }

    *bi++ = mx;
  }
}


/*
 * Expected ip, return (via back insert iterator)
 * all host names associated with this ip; nothing will be added in case of
 * failure.
 */
template <class BackInsertIterator>
void name_by_ip( uint32_t ip, BackInsertIterator bi )
{
  std::stringstream s;

  s << (0xff & (ip >> 24)) << '.'
    << (0xff & (ip >> 16)) << '.'
    << (0xff & (ip >> 8))  << '.'
    << (0xff & ip)
    << ".in-addr.arpa";

  unsigned char bufa[4096];
  int len = res_search( s.str().c_str(), ns_c_in, ns_t_ptr, bufa, 4096 );

  if ( len <= 0 ) {
    return;
  }

  details::ns_answer answer( bufa, len );
  for ( typename details::ns_answer::iterator j = answer.begin(); j != answer.end(); ++j ) {
    if ( ns_rr_type(*j) == ns_t_ptr ) {
      const unsigned char *rdata = ns_rr_rdata(*j);
      const unsigned char *edata = rdata + ns_rr_rdlen(*j);

      std::string name = details::decode_string( rdata, edata, bufa );

      // The name ordinary returned as name.domain.com. (dot at end).
      // Remove it.
      std::string::size_type pos = name.rfind( '.' );

      if ( pos != std::string::npos ) {
        if ( pos == (name.size() - 1) ) {
          name.erase( pos, 1 );
        }
      }
      *bi++ = name;
    }
  }
}

template <class PMF>
void mh_ip( const std::string& domain, typename PMF::reference_class_type O, typename PMF::pmf_type A );

namespace details {

class resolve
{
  public:
    typedef int hits_type;

    struct domain_results {
        priority_ip ips;
        hits_type hits;
    };

    resolve()
      {
        if ( (_res.options & RES_INIT) == 0 ) {
          std::cerr << "res_init" << std::endl;
          res_init();
        } else {
          std::cerr << "res_init was called" << std::endl;
        }
      }

  public:
    // typedef unsigned id_type;
    // typedef std::map<std::string,id_type> string_map_type;


    // typedef std::set<std::string> string_map_type;
    // typedef string_map_type::iterator id_type;
    // typedef std::map<id_type,domain_results> container_type;
    // typedef container_type::iterator domain_results_ref_type;
    // typedef std::priority_queue<domain_results_ref_type> hits_container_type;
    // typedef std::map<uint32_t,id_type> reverse_ip_container_type;

    typedef std::map<std::string,domain_results> container_type;
    typedef std::map<uint32_t,std::string> reverse_ip_container_type;

    // static container_type& cache()
    //  { return r; }

    priority_ip& operator [](const std::string& );

  private:
//    string_map_type     str_map;
    container_type      r;
//    hits_container_type hits_map;

    // static id_type _last_id;
};
#if 0
inline bool operator <( const resolve::domain_results_ref_type& l, const resolve::domain_results_ref_type& r )
{
  return l->second.hits < r->second.hits;
}
#endif

extern resolve _resolver;

} // namespace details

template <class T>
class forward
{
  public:
    forward( const T& );

    std::string route( const std::string& address );

  private:
    struct to {
        bool with_mx;
        bool with_re;
        int port;     // active only when with_mx == false
        std::string name;
        boost::regex re; // template for search
    };

  private:

    typedef std::pair<std::string,to> record_type;
    typedef std::vector<record_type>  container_type;

    container_type fwd;
};

template <class T>
forward<T>::forward( const T& c )
{
  static boost::regex REfwdtoken( "[[:space:]]*(.*)[[:space:]]+((\\[([[:upper:][:lower:][:digit:]._]+)(\\:([[:digit:]]+))?\\])|([[:upper:][:lower:][:digit:]._]+))[[:space:]]*" );


  for ( typename T::const_iterator i = c.begin(); i != c.end(); ++i ) {
    // std::cerr << *i << std::endl;
    boost::smatch ma;
    if ( boost::regex_match( *i, ma, REfwdtoken ) ) {
      // std::string left( ma[1] );
      // std::string port( ma[6].length() > 0 ? ma[6] : std::string( "25" ) );
      // bool with_mx = ma[3].length() > 0 ? false : true;
      // std::string name( with_mx ? ma[7] : ma[4] );

      std::string re_string = ma[1];

      if ( re_string == "any" ) { // process: 'any [localhost:1025]'
        re_string = "*";
      }

      fwd.push_back( std::make_pair( re_string, to() ) );

      if ( ma[3].length() == 0 ) {
        fwd.back().second.with_mx = true;
        fwd.back().second.port = 25;
        fwd.back().second.name = ma[7];
      } else {
        fwd.back().second.with_mx = false;
        std::stringstream s( ma[6].length() > 0 ? ma[6] : std::string( "25" ) );
        s >> fwd.back().second.port;
        fwd.back().second.name = ma[4];
      }

      fwd.back().second.with_re = false;

      std::string::size_type p = 0;

      // first escape dots
      while ( (p = re_string.find( '.', p )) != std::string::npos ) {
        re_string.insert( p, 1, '\\' );
        p += 2;
      }

      // Is '*' present? Replace it by '.*'.
      p = 0;
      while ( (p = re_string.find( '*', p )) != std::string::npos ) {
        re_string.insert( p, 1, '.' );
        p += 2;
        fwd.back().second.with_re = true;
      }

      // Is '?' present? Replace it by '.'.
      p = 0;
      while ( (p = re_string.find( '?', p )) != std::string::npos ) {
        re_string[p] = '.';
        ++p;
        fwd.back().second.with_re = true;
      }

      fwd.back().second.re = re_string;

#if 0
      std::string left = i->substr( ma[1].first - i->begin(), ma[1].second - ma[1].first );
      std::string right = i->substr( ma[2].first - i->begin(), ma[2].second - ma[1].first );
#else
#if 0
      std::cerr << "Match: \n";
      for ( unsigned j = 0; j < ma.size(); ++j ) {
        std::cerr << j << "{" << ma[j] << "}" << std::endl;
      }
      std::cerr << "-------\n";
#endif
#endif
    } else {
      // syntax error
    }
  }
}

template <class T>
std::string forward<T>::route( const std::string& address )
{
  std::string::size_type p = address.find( '@' );

  std::string domain = (p != std::string::npos) ? address.substr( p + 1 ) : address;
  boost::smatch ma;

  for ( typename container_type::const_iterator i = fwd.begin(); i != fwd.end(); ++i ) {
    // compare only domain part, if no '@'
    const std::string& domain_or_address = (i->first.find( '@' ) == std::string::npos) ? domain : address;

    bool match = false;

    if ( i->second.with_re ) {
      if ( boost::regex_match( domain_or_address, ma, i->second.re ) ) {
        match = true;
      }
    } else if ( i->first == domain_or_address ) {
      match = true;
    }

    if ( match ) {
      std::stringstream s;

      s << i->second.name << ' '
        << i->second.with_mx << ' ' << i->second.port;

      return s.str();
    }
  }

  return domain + " 1 25";
}

template <class PMF>
void mh_ip( const std::string& domain, typename PMF::reference_class_type O, typename PMF::pmf_type A )
{
  // details::priority_ip& ips = details::resolve::cache()[domain];

  std::stringstream parse( domain );

  std::string str_domain;
  bool with_mx = true;
  int port;

  parse >> str_domain >> with_mx >> port;

  if ( parse.fail() ) {
    throw std::invalid_argument( domain );
  }

  boost::smatch ma;
  static boost::regex REip( "[[:space:]]*([[:digit:]._]+)[[:space:]]*" );

  if ( boost::regex_match( str_domain, ma, REip ) ) {
    int tmp;
    char dot;
    union {
     uint32_t ip;
     char byte[4];
    } uip;

    std::stringstream s_ip( str_domain );

    s_ip >> tmp >> dot;
    if ( s_ip.fail() ) {
      throw std::invalid_argument( domain );
    }
    uip.byte[0] = tmp;

    s_ip >> tmp >> dot;
    if ( s_ip.fail() ) {
      throw std::invalid_argument( domain );
    }
    uip.byte[1] = tmp;

    s_ip >> tmp >> dot;
    if ( s_ip.fail() ) {
      throw std::invalid_argument( domain );
    }
    uip.byte[2] = tmp;

    s_ip >> tmp;
    if ( s_ip.fail() ) {
      throw std::invalid_argument( domain );
    }
    uip.byte[3] = tmp;

    try {
      (O.*A)( uip.ip, htons(port) );
      return;
    }
    catch ( domain_processing_error& e ) {
      throw domain_processing_error( "permanent errors during domain processing" );
    }
  }

  details::priority_ip& ips = details::_resolver[str_domain];
  uint32_t ip_requested = details::priority_ip::bad_ip;

  if ( ips.empty() ) {
    // try to take MXs for domain
    unsigned char buf[4096];
    int len;
    if ( with_mx ) {
      len = res_search( str_domain.c_str(), ns_c_in, ns_t_mx, buf, 4096 );
    }
    if ( with_mx && (len > 0) ) {
#if 0
      cerr << len << endl;
      for ( int i = 0; i < len; ++i ) {
        if ( i % 10 == 0 ) {
          cerr << ' ';
        }
        cerr << hex << setfill('0') << setw(2) << (unsigned)buf[i];
      }
      cerr << dec << endl;
      // cerr << buf << endl;
#endif
      ns_msg answer;
      details::x_ns_initparse( buf, len, &answer );
      int nr = ns_msg_count( answer, ns_s_an );
      // cerr <<  nr << endl;
      if ( nr == 0 ) {
        // hmm, len was grater than zero, but number of records 0; hmm...
        // nevertheless, let's try to resolve ip for 'domain'
        std::list<uint32_t> l;
        ip_by_name( str_domain, back_inserter(l));
        int priority = 100000; // > 2^16
        for ( std::list<uint32_t>::const_iterator li = l.begin(); li != l.end(); ++li ) {
          ips.push( priority, *li );
          // cerr << "Result: " << domain << " " << priority << " "
          //     << hex << *li << dec << endl;
        }
      } else {
        ns_rr rr;
        for ( int i = 0; i < nr; ++i ) {
          details::x_ns_parserr( &answer, ns_s_an, i, &rr );
          // cerr << "Type: " << ns_rr_type(rr) << endl;
          // cerr << "TTL: " << ns_rr_ttl(rr) << endl;
          // cerr << "Class: " << ns_rr_class(rr) << endl;
          // cerr << "Name: " << ns_rr_name(rr) << endl; // \0?

          uint16_t priority = ntohs(*(uint16_t*)ns_rr_rdata(rr));
          // cerr << "Priority: " << priority << endl;

          const unsigned char *rdata = ns_rr_rdata(rr);
          const unsigned char *edata = rdata + ns_rr_rdlen(rr);
          rdata += sizeof(uint16_t);

          std::string mx = details::decode_string( rdata, edata, buf );

          // The name ordinary returned as name.domain.com. (dot at end).
          // Remove it.
          std::string::size_type pos = mx.rfind( '.' );

          if ( pos != std::string::npos ) {
            if ( pos == (mx.size() - 1) ) {
              mx.erase( pos, 1 );
            }
          }

          // cerr << mx << endl;
          std::list<uint32_t> l;
          ip_by_name( mx, back_inserter(l));
          for ( std::list<uint32_t>::const_iterator li = l.begin(); li != l.end(); ++li ) {
            ips.push( priority, *li );
            // cerr << "Result: " << domain << " " << priority << " "
            //      << hex << *li << dec << endl;
          }
        }
      }
    } else { // let's try to resolve ip for 'domain' (no MXs for 'domain')
      std::list<uint32_t> l;
      ip_by_name( str_domain, back_inserter(l));
      int priority = 100000; // > 2^16
      for ( std::list<uint32_t>::const_iterator li = l.begin(); li != l.end(); ++li ) {
        ips.push( priority, *li );
        // cerr << "Result: " << domain << " " << priority << " "
        //      << hex << *li << dec << endl;
      }
    }
  }

  if ( !ips.empty() ) {
    for ( int i = ips.size(); i > 0; --i ) {
      ip_requested = ips.top();
      try {
        if ( with_mx ) {
          (O.*A)( ip_requested, htons(25) );
        } else {
          (O.*A)( ip_requested, htons(port) );
        }
        return;
      }
      catch ( domain_processing_error& e ) {
        ips.penalty();
      }
      // if ( /* do_somtyhing( ip_requested ) == fail */ true ) {
      //  ips.penalty();
      // }
    }

    throw domain_processing_error( "permanent errors during domain processing" );
  }

  throw ip_resolving_error( "domain name unresolved" );
}

} // namespace resolver

#endif // __resolver_res_ip_h

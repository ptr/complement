// -*- C++ -*- Time-stamp: <05/11/17 01:22:17 ptr>

#include <boost/test/unit_test.hpp>

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>

#include <resolver/res_ip.h>

#include <netinet/in.h>

using namespace std;

static bool v = false;

static string domain = "homecredit.ru 1 25";
static string host = "mxs.homecredit.ru 1 25";
static string badhost = "bogus.name.err 1 25";
static string hosts_host = "hosts.check.dom 1 25"; // /etc/hosts: 127.0.0.1       localhost hosts.check.dom

class Sample
{
  public:

    // expected throw: domain_processing_error and ip_resolving_error
    void process0( uint32_t a, int port ) throw( runtime_error );
    void process1( uint32_t a, int port ) throw( runtime_error );
    void process2( uint32_t a, int port ) throw( runtime_error );
    void process3( uint32_t a, int port ) throw( runtime_error );
    void hosts( uint32_t a, int port ) throw( runtime_error );
};

void Sample::process0( uint32_t a, int port ) throw( runtime_error )
{
  BOOST_CHECK( v == false );
  v = true;
}

void Sample::process1( uint32_t a, int port ) throw( runtime_error )
{
  // 62.141.69.98 or 217.69.216.163
  BOOST_CHECK( a == htonl( 0x3e8d4562 ) || a == htonl( 0xd945d8a3 ) );
}

void Sample::process2( uint32_t a, int port ) throw( runtime_error )
{
  // 62.141.69.98 or 217.69.216.163
  BOOST_CHECK( a == htonl( 0x3e8d4562 ) || a == htonl( 0xd945d8a3 ) );
}

void Sample::process3( uint32_t a, int port ) throw( runtime_error )
{
  BOOST_ERROR( "Shouldn't be called---exception expected" );
}

void Sample::hosts( uint32_t a, int port ) throw( runtime_error )
{
  BOOST_CHECK( a == htonl( 0x7f000001 ) ); // 127.0.0.1
}

void test_basic()
{
  Sample s;
  
  try {
    resolver::mh_ip<resolver::member_function<Sample> >( domain, s, &Sample::process0 );
    BOOST_CHECK( v == true );
  }
  catch ( const runtime_error& e ) {
    BOOST_ERROR( e.what() );
  }

  try {
     resolver::mh_ip<resolver::member_function<Sample> >( domain, s, &Sample::process1 );
  }
  catch ( const runtime_error& e ) {
    BOOST_ERROR( e.what() );
  }

  try {
    resolver::mh_ip<resolver::member_function<Sample> >( host, s, &Sample::process2 );
  }
  catch ( const runtime_error& e ) {
    BOOST_ERROR( e.what() );
  }

  try {
    resolver::mh_ip<resolver::member_function<Sample> >( badhost, s, &Sample::process3 );
    BOOST_ERROR( "Exception expected" );
  }
  catch ( const runtime_error& e ) {
    try {
      BOOST_CHECK( typeid(dynamic_cast<const resolver::ip_resolving_error&>(e)) == typeid(resolver::ip_resolving_error) );
    }
    catch ( const bad_cast& ) {
      BOOST_ERROR( "Expected resolver::ip_resolving_error exception" );
    }
  }
}

void test_name_by_ip()
{
  vector<string> s;
  // 217.69.216.163
  resolver::name_by_ip( htonl( 0xd945d8a3 ), back_inserter( s ) );

  BOOST_CHECK( s.size() == 1 );
  BOOST_CHECK( *s.begin() == "mx1.homecredit.ru" );

  // for ( vector<string>::iterator i = s.begin(); i != s.end(); ++i ) {
  //   cerr << *i << endl;
  // }
}

union BO {
    uint32_t i;
    char b[4];
};


void test_local()
{
  vector<uint32_t> a;
  resolver::ip_by_name( "localhost", back_inserter( a ) );

  BOOST_CHECK( a.size() == 1 );
  // cerr << hex << *a.begin() << dec << endl;
  BO bo;
  bo.i = 1;

  if ( bo.b[0] == 1 ) { // LSB
    BOOST_CHECK( *a.begin() == 0x100007f );
  } else { //MSB
    BOOST_CHECK( *a.begin() == 0x7f000001 );
  }

#if 0
  // don't check it, we see something like 'localhost.avp.ru'
  vector<string> s;
  if ( bo.b[0] == 1 ) { // LSB
    resolver::name_by_ip( 0x100007f, back_inserter( s ) );
  } else { // MSB
    resolver::name_by_ip( 0x7f000001, back_inserter( s ) );
  }

  BOOST_CHECK( s.size() > 0 );

  cerr << *s.begin() << endl;
  // BOOST_CHECK( *s.begin() == "localhost" );
#endif
}

void test_mroute()
{
  vector<string> v;

  v.push_back( "avp.ru mail.avp.ru" );
  v.push_back( "test.avp.ru [mail2.avp.ru]" );
  v.push_back( "support.avp.ru [mail2.avp.ru:1025]" );
  v.push_back( "bad1.avp.ru bad.avp.ru:1025" ); // not allowed (just ignored yet)
  v.push_back( "*.domain.ru mail3.avp.ru" );
  v.push_back( "BLUser@server?.hosting.ru mail4.avp.ru" );

  resolver::forward<vector<string> > fw( v );

  string r = fw.route( "user@avp.ru" );

  BOOST_CHECK( r == "mail.avp.ru 1 25" );

  r = fw.route( "BLUser@test.avp.ru" );

  BOOST_CHECK( r == "mail2.avp.ru 0 25" );

  r = fw.route( "BLUser@support.avp.ru" );

  BOOST_CHECK( r == "mail2.avp.ru 0 1025" );

  r = fw.route( "BLUser@bad1.avp.ru" );

  BOOST_CHECK( r == "bad1.avp.ru 1 25" );

  r = fw.route( "BLUser@other.avp.ru" );

  BOOST_CHECK( r == "other.avp.ru 1 25" );

  r = fw.route( "BLUser@host.domain.ru" );

  BOOST_CHECK( r == "mail3.avp.ru 1 25" );

  r = fw.route( "BLUser@server2.hosting.ru" );

  BOOST_CHECK( r == "mail4.avp.ru 1 25" );

  vector<string> v2;
  v2.push_back( "any [localhost:1025]" );

  resolver::forward<vector<string> > fw2( v2 );

  r = fw2.route( "user@something.else.info" );

  BOOST_CHECK( r == "localhost 0 1025" );
}

void test_hosts()
{
  Sample s;
  
  try {
    resolver::mh_ip<resolver::member_function<Sample> >( hosts_host, s, &Sample::hosts );
  }
  catch ( const runtime_error& e ) {
    BOOST_ERROR( e.what() );
  }
}

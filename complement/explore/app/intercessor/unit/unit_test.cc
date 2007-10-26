// -*- C++ -*- Time-stamp: <07/03/07 16:38:24 ptr>

#include <boost/test/unit_test.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <iostream>

#include <sstream>
#include <fstream>
#include <iterator>
#include <unistd.h>

#include "http.h"
#include "intercessor.h"
#include "server.h"

#include "dummy_srv.h"
#include <sockios/sockmgr.h>
#include <mt/lfstream.h>

using namespace boost::unit_test_framework;

using namespace std;
using namespace http;

boost::filesystem::path dir( boost::filesystem::initial_path() );
unsigned rq_timeout = 3;

void http_header_io_test()
{
  header h;
  string rq = "Content-Length: 100\r\n";
  stringstream s( rq );

  s >> h;

  BOOST_CHECK( !s.fail() );
  BOOST_CHECK( h.key() == "Content-Length" );
  BOOST_CHECK( h.value() == "100" );

  stringstream o;

  o << h;

  BOOST_CHECK( !o.fail() );
  BOOST_CHECK( o.str() == "Content-Length: 100\r\n" );
}

void http_header_sp_test()
{
  header h;
  string rq = "Header: fields here \r\n";
  stringstream s( rq );

  s >> h;

  BOOST_CHECK( !s.fail() );
  BOOST_CHECK( h.key() == "Header" );
  BOOST_CHECK( h.value() == "fields here" );

  stringstream o;

  o << h;

  BOOST_CHECK( !o.fail() );
  BOOST_CHECK( o.str() == "Header: fields here\r\n" );
}

void http_command_test()
{
  string cmd = "GET http://myhost.com HTTP/1.1\r\n";
  stringstream s( cmd );
  command c;

  s >> c;

  BOOST_CHECK( !s.fail() );
  BOOST_CHECK( c.value() == command::GET );
  BOOST_CHECK( c.URL() == "http://myhost.com" );
  BOOST_CHECK( c.protocol() == command::HTTP11 );

  stringstream o;

  o << c;

  BOOST_CHECK( !o.fail() );
  BOOST_CHECK( o.str() == "GET http://myhost.com HTTP/1.1\r\n" );
}

void http_base_response_test()
{
  string rs = "HTTP/1.1 202 Accepted\r\n";
  stringstream s( rs );
  base_response r;

  s >> r;

  BOOST_CHECK( !s.fail() );
  BOOST_CHECK( r.code() == 202 );
  BOOST_CHECK( r.protocol() == command::HTTP11 );

  stringstream o;

  o << r;

  BOOST_CHECK( !o.fail() );
  BOOST_CHECK( o.str() == "HTTP/1.1 202 Accepted\r\n" );
}

void http_request_test()
{
  string rq = "GET /index.html HTTP/1.1\r\n"
              "Host: myhost.com\r\n"
              "X-Header: test\r\n"
              "\r\n";
  stringstream s( rq );
  request r;

  s >> r;

  BOOST_CHECK( !s.fail() );
  BOOST_CHECK( r.head().protocol() == command::HTTP11 );
  BOOST_CHECK( r.head().value() == command::GET );
  BOOST_CHECK( r.head().URL() == "/index.html" );
  BOOST_CHECK( r.headers().size() == 2 );
  BOOST_CHECK( r.headers().begin()->key() == "Host" );
  BOOST_CHECK( r.headers().begin()->value() == "myhost.com" );

  BOOST_CHECK( r.search( "Host" ) == r.headers().begin() );
}

void http_response_test()
{
  string rq = "HTTP/1.1 202 Accepted\r\n"
              "Content-Length: 100\r\n"
              "\r\n";
  stringstream s( rq );
  response r;

  s >> r;

  BOOST_CHECK( !s.fail() );
  BOOST_CHECK( r.head().protocol() == command::HTTP11 );
  BOOST_CHECK( r.head().code() == 202 );
  BOOST_CHECK( r.headers().size() == 1 );
  BOOST_CHECK( r.headers().begin()->key() == "Content-Length" );
  BOOST_CHECK( boost::lexical_cast<int>( r.headers().begin()->value() ) == 100 );

  BOOST_CHECK( r.search( "Content-Length" ) == r.headers().begin() );
}

void files_content( bool remove = true )
{
  {
    ifstream file( "/tmp/test.xxx" );

    BOOST_CHECK( file.good() );

    string str1;
    // file.lock_sh();
    getline( file, str1 );
    // file.unlock();

    BOOST_CHECK( str1 == "1234567" );
  }

    // cerr << "'" << str1 << "'" << endl;
  {
    ifstream headers( "/tmp/test.xxx.head" );
    istreambuf_iterator<char> istr( headers.rdbuf() );

    string str1;

    // headers.lock_sh();
    while ( istr != istreambuf_iterator<char>() ) {
      str1 += *istr;
      ++istr;
    }
    // headers.unlock();

    string ref_string = "HTTP/1.1 200 Ok\r\n"
                        "Content-Length: 8\r\n"
                        "\r\n";

    BOOST_CHECK( str1 == ref_string );
  }

  if ( remove ) {
    ::unlink( "/tmp/test.xxx" );
    ::unlink( "/tmp/test.xxx.head" );
  }
}

void intercessor_test()
{
  string rq = "GET http://localhost:8090/test.php?var=123 HTTP/1.1\r\n"
              "Host: localhost:8090\r\n"
              "X-API-ReportFileName: test.xxx\r\n"
              "\r\n";
  
  stringstream s( rq );
  request r;

  s >> r;

  BOOST_CHECK( r.search( "X-API-ReportFileName" ) != r.headers().end() );
  BOOST_CHECK( r.search( "X-API-ReportFileName" )->value() == "test.xxx" );

  test::DummyHttpSrv::cnd.set( false );

  sockmgr_stream_MP<test::DummyHttpSrv> mgr( 8090 );
  mgr.setoptions( sock_base::so_keepalive, true );
  mgr.setoptions( sock_base::so_reuseaddr, true );

  BOOST_CHECK( mgr.good() );

#if 0
  { // Dummy connect, to be sure that mgr already listen port
    sockstream first_dummy( "localhost", 8080 );

    first_dummy << "dummy string" << endl;
  
    BOOST_CHECK( !first_dummy.fail() );
  }

  test::DummyHttpSrv::cnd.try_wait();

  test::DummyHttpSrv::cnd.set( false );
#endif

  intr::Intercessor intercessor;

  stem::Event_base<intr::httprq> ev( 0x702 );
  ev.dest( intercessor.self_id() );
  ev.value().rq = r;
  intercessor.Send( ev );

  test::DummyHttpSrv::cnd.try_wait();

  mgr.close();
  mgr.wait();

  files_content();
}

void http_processor_test()
{
  {
    intr::Intercessor intercessor(0);
    sockmgr_stream_MP<intr::IncomeHttpRqProcessor> mgr( 8091 );

    // mgr.setoptions( sock_base::so_keepalive, true );
    mgr.setoptions( sock_base::so_reuseaddr, true );

    sockmgr_stream_MP<test::DummyHttpSrv> dummy_mgr( 8092 );

    // dummy_mgr.setoptions( sock_base::so_keepalive, true );
    dummy_mgr.setoptions( sock_base::so_reuseaddr, true );

#if 0
    for ( int i = 0; i < 10; ++i ) {
      test::DummyHttpSrv::cnd.set( false );
      { // Dummy connect, to be sure that mgr already listen port
        sockstream first_dummy( "localhost", 8092 );

        first_dummy << "dummy string" << endl;
  
        cerr << "{" << i << ", " << first_dummy.good() << endl;
        BOOST_CHECK( !first_dummy.fail() );
      }
      test::DummyHttpSrv::cnd.try_wait();
    }

#endif
    BOOST_CHECK( mgr.good() );
    BOOST_CHECK( dummy_mgr.good() );

    test::DummyHttpSrv::cnd.set( false );

    system( "curl -x localhost:8091 -H \"X-API-ReportFileName: test.xxx\" http://localhost:8092/test.php?var=123" );

    test::DummyHttpSrv::cnd.try_wait();

    dummy_mgr.close();
    dummy_mgr.wait();

    mgr.close();
    mgr.wait();
  }

  files_content();
}

void http_processor_post_test()
{
  {
    intr::Intercessor intercessor(0);
    sockmgr_stream_MP<intr::IncomeHttpRqProcessor> mgr( 8091 );
    // mgr.setoptions( sock_base::so_keepalive, true );
    // mgr.setoptions( sock_base::so_reuseaddr, true );

    sockmgr_stream_MP<test::DummyHttpSrv> dummy_mgr( 8092 );

    // dummy_mgr.setoptions( sock_base::so_keepalive, true );
    // dummy_mgr.setoptions( sock_base::so_reuseaddr, true );

#if 0
    test::DummyHttpSrv::cnd.set( false );
    { // Dummy connect, to be sure that mgr already listen port
      sockstream first_dummy( "localhost", 8080 );

      first_dummy << "dummy string" << endl;
  
      BOOST_CHECK( !first_dummy.fail() );
    }
    test::DummyHttpSrv::cnd.try_wait();
#endif
    BOOST_CHECK( mgr.good() );
    BOOST_CHECK( dummy_mgr.good() );

    test::DummyHttpSrv::cnd.set( false );

    system( "curl -x localhost:8091 -H \"X-API-ReportFileName: test.xxx\" \
-d 'xmlrequest=<?xml version=\"1.0\"?>\
<RWRequest><REQUEST domain=\"network\" service=\"ComplexReport\" nocache=\"n\" \
contact_id=\"1267\" entity=\"1\" filter_entity_id=\"1\" \
clientName=\"ui.ent\"><ROWS><ROW type=\"group\" priority=\"1\" ref=\"entity_id\" \
includeascolumn=\"n\"/><ROW type=\"group\" priority=\"2\" \
ref=\"advertiser_line_item_id\" includeascolumn=\"n\"/><ROW type=\"total\"/></ROWS><COLUMNS><COLUMN \
ref=\"advertiser_line_item_name\"/><COLUMN ref=\"seller_imps\"/><COLUMN \
ref=\"seller_clicks\"/><COLUMN ref=\"seller_convs\"/><COLUMN \
ref=\"click_rate\"/><COLUMN ref=\"conversion_rate\"/><COLUMN ref=\"roi\"/><COLUMN \
ref=\"network_revenue\"/><COLUMN ref=\"network_gross_cost\"/><COLUMN \
ref=\"network_gross_profit\"/><COLUMN ref=\"network_revenue_ecpm\"/><COLUMN \
ref=\"network_gross_cost_ecpm\"/><COLUMN \
ref=\"network_gross_profit_ecpm\"/></COLUMNS><FILTERS><FILTER ref=\"time\" \
macro=\"yesterday\"/></FILTERS></REQUEST></RWRequest>' http://localhost:8092/test.php" );

    test::DummyHttpSrv::cnd.try_wait();

    dummy_mgr.close();
    dummy_mgr.wait();

    mgr.close();
    mgr.wait();
  }

  files_content();
}

void http_processor_external_post_test()
{
  {
    intr::Intercessor intercessor(0);
    sockmgr_stream_MP<intr::IncomeHttpRqProcessor> mgr( 8095 );
    // mgr.setoptions( sock_base::so_keepalive, true );
    // mgr.setoptions( sock_base::so_reuseaddr, true );
    if ( !mgr.good() ) {
      cerr << "Not good" << endl;
      return;
    }

    system( "curl -x localhost:8095 -H \"X-API-ReportFileName: test.xxx\" \
-d 'xmlrequest=<?xml version=\"1.0\"?>\
<RWRequest><REQUEST domain=\"network\" service=\"ComplexReport\" nocache=\"n\" \
contact_id=\"1267\" entity=\"1\" filter_entity_id=\"1\" \
clientName=\"ui.ent\"><ROWS><ROW type=\"group\" priority=\"1\" ref=\"entity_id\" \
includeascolumn=\"n\"/><ROW type=\"group\" priority=\"2\" \
ref=\"advertiser_line_item_id\" includeascolumn=\"n\"/><ROW type=\"total\"/></ROWS><COLUMNS><COLUMN \
ref=\"advertiser_line_item_name\"/><COLUMN ref=\"seller_imps\"/><COLUMN \
ref=\"seller_clicks\"/><COLUMN ref=\"seller_convs\"/><COLUMN \
ref=\"click_rate\"/><COLUMN ref=\"conversion_rate\"/><COLUMN ref=\"roi\"/><COLUMN \
ref=\"network_revenue\"/><COLUMN ref=\"network_gross_cost\"/><COLUMN \
ref=\"network_gross_profit\"/><COLUMN ref=\"network_revenue_ecpm\"/><COLUMN \
ref=\"network_gross_cost_ecpm\"/><COLUMN \
ref=\"network_gross_profit_ecpm\"/></COLUMNS><FILTERS><FILTER ref=\"time\" \
macro=\"yesterday\"/></FILTERS></REQUEST></RWRequest>' http://ses0316:8080/rpt" );

    mgr.wait();
  }
}

void intercessor_negative()
{
  string rq = "GET http://localhost:8092/test.php?var=123 HTTP/1.1\r\n"
              "Host: localhost:8092\r\n"
              "Proxy-Connection: localhost:8091\r\n"
              "X-API-ReportFileName: test.xxx\r\n"
              "\r\n";

  {
    intr::Intercessor intercessor(0);
    sockmgr_stream_MP<intr::IncomeHttpRqProcessor> mgr( 8091 );
    sockmgr_stream_MP<test::DummyHttpSrvNeg> dummy_mgr( 8092 );

    BOOST_CHECK( mgr.good() );
    BOOST_CHECK( dummy_mgr.good() );

    test::DummyHttpSrvNeg::cnd.set( false );

    {
      string rs;
      sockstream client( "localhost", 8091 );
      BOOST_CHECK( client.good() );

      (client << rq).flush();
      getline( client, rs );

      BOOST_CHECK( rs == "HTTP/1.1 410 Gone\r" );
    }

    test::DummyHttpSrvNeg::cnd.try_wait();

    dummy_mgr.close();
    dummy_mgr.wait();

    mgr.close();
    mgr.wait();
  }
}

test_suite *init_unit_test_suite( int argc, char** const argv )
{
  dir = boost::filesystem::system_complete( boost::filesystem::path( "/tmp",  boost::filesystem::native ) );

  test_suite *ts = BOOST_TEST_SUITE( "intercessor test" );

  ts->add( BOOST_TEST_CASE( &http_header_io_test ) );
  ts->add( BOOST_TEST_CASE( &http_header_sp_test ) );
  ts->add( BOOST_TEST_CASE( &http_command_test ) );
  ts->add( BOOST_TEST_CASE( &http_base_response_test ) );
  ts->add( BOOST_TEST_CASE( &http_request_test ) );
  ts->add( BOOST_TEST_CASE( &http_response_test ) );
  ts->add( BOOST_TEST_CASE( &intercessor_test ), 0, 7 );
  ts->add( BOOST_TEST_CASE( &http_processor_test ), 0, 7 );
  ts->add( BOOST_TEST_CASE( &http_processor_post_test ), 0, 7 );
  // ts->add( BOOST_TEST_CASE( &http_processor_external_post_test ), 0, 60 );
  ts->add( BOOST_TEST_CASE( &intercessor_negative ), 0, 7 );
 
  return ts;
}

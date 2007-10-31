// -*- C++ -*- Time-stamp: <07/03/07 16:38:24 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "intercessor_test.h"
#include <boost/lexical_cast.hpp>

#include <string>
#include <iostream>

#include <sstream>
#include <fstream>
#include <iterator>
#include <unistd.h>

#include <net/http.h>
#include "intercessor.h"
#include "server.h"

#include "dummy_srv.h"
#include <sockios/sockmgr.h>
#include <mt/lfstream.h>

boost::filesystem::path dir( boost::filesystem::initial_path() );
unsigned rq_timeout = 3;

using namespace std;
using namespace http;

void files_content( bool remove = true )
{
  {
    ifstream file( "/tmp/test.xxx" );

    EXAM_CHECK_ASYNC( file.good() );

    string str1;
    // file.lock_sh();
    getline( file, str1 );
    // file.unlock();

    EXAM_CHECK_ASYNC( str1 == "1234567" );
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

    EXAM_CHECK_ASYNC( str1 == ref_string );
  }

  if ( remove ) {
    ::unlink( "/tmp/test.xxx" );
    ::unlink( "/tmp/test.xxx.head" );
  }
}

int EXAM_IMPL(intercessor_test::base)
{
  dir = boost::filesystem::system_complete( boost::filesystem::path( "/tmp",  boost::filesystem::native ) );

  string rq = "GET http://localhost:8090/test.php?var=123 HTTP/1.1\r\n"
              "Host: localhost:8090\r\n"
              "X-API-ReportFileName: test.xxx\r\n"
              "\r\n";

  stringstream s( rq );
  request r;

  s >> r;

  EXAM_CHECK( r.search( "X-API-ReportFileName" ) != r.headers().end() );
  EXAM_CHECK( r.search( "X-API-ReportFileName" )->value() == "test.xxx" );

  test::DummyHttpSrv::cnd.set( false );

  sockmgr_stream_MP<test::DummyHttpSrv> mgr( 8090 );
  mgr.setoptions( sock_base::so_keepalive, true );
  mgr.setoptions( sock_base::so_reuseaddr, true );

  EXAM_CHECK( mgr.good() );

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

  return EXAM_RESULT;
}

int EXAM_IMPL(intercessor_test::processor)
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
    EXAM_CHECK( mgr.good() );
    EXAM_CHECK( dummy_mgr.good() );

    test::DummyHttpSrv::cnd.set( false );

    system( "curl -x localhost:8091 -H \"X-API-ReportFileName: test.xxx\" http://localhost:8092/test.php?var=123" );

    test::DummyHttpSrv::cnd.try_wait();

    dummy_mgr.close();
    dummy_mgr.wait();

    mgr.close();
    mgr.wait();
  }

  files_content();

  return EXAM_RESULT;
}

int EXAM_IMPL(intercessor_test::processor_post)
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
    EXAM_CHECK( mgr.good() );
    EXAM_CHECK( dummy_mgr.good() );

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
    dummy_mgr.wait();

    mgr.close();
    mgr.wait();
  }

  files_content();

  return EXAM_RESULT;
}

int EXAM_IMPL(intercessor_test::processor_external_post)
{
  {
    intr::Intercessor intercessor(0);
    sockmgr_stream_MP<intr::IncomeHttpRqProcessor> mgr( 8095 );
    // mgr.setoptions( sock_base::so_keepalive, true );
    // mgr.setoptions( sock_base::so_reuseaddr, true );
    EXAM_REQUIRE( mgr.good() );

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

  return EXAM_RESULT;
}

int EXAM_IMPL(intercessor_test::negative)
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

    EXAM_CHECK( mgr.good() );
    EXAM_CHECK( dummy_mgr.good() );

    test::DummyHttpSrvNeg::cnd.set( false );

    {
      string rs;
      sockstream client( "localhost", 8091 );
      EXAM_CHECK( client.good() );

      (client << rq).flush();
      getline( client, rs );

      EXAM_CHECK( rs == "HTTP/1.1 410 Gone\r" );
    }

    test::DummyHttpSrvNeg::cnd.try_wait();

    dummy_mgr.close();
    dummy_mgr.wait();

    mgr.close();
    mgr.wait();
  }

  return EXAM_RESULT;
}


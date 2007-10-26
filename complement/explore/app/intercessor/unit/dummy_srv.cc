// -*- C++ -*- Time-stamp: <07/03/07 15:54:00 ptr>

#include "dummy_srv.h"

#include <sockios/sockmgr.h>

#include <list>
#include <string>
#include <stdexcept>
#include <iostream>
#include <boost/regex.hpp>

#include <boost/test/unit_test.hpp>

#include "http.h"

namespace test {

using namespace std;
using namespace xmt;

condition DummyHttpSrv::cnd;

DummyHttpSrv::DummyHttpSrv( std::sockstream& )
{
  // cerr << "DummyHttpSrv::DummyHttpSrv" << endl;
}

void DummyHttpSrv::connect( std::sockstream& s )
{
  // cerr << "DummyHttpSrv::connect " << pthread_self() << endl;

  BOOST_CHECK( s.good() );

  http::request rq;

  s >> noskipws >> http::body( true ) >> rq;

  // BOOST_CHECK( s.good() );

  if ( rq.head().value() == http::command::GET ) {
    http::response rs;
    rs.head().code( 200 );
    rs.body() = "1234567\n";

    (s << http::body( true ) << rs).flush();

    BOOST_CHECK( s.good() );

  } else if ( rq.head().value() == http::command::POST ) {
    http::response rs;
    rs.head().code( 200 );
    rs.body() = "1234567\n";

    (s << http::body( true ) << rs).flush();
    
    BOOST_CHECK( s.good() );
  }
}

void DummyHttpSrv::close()
{
  // cerr << "DummyHttpSrv::close" << endl;
  cnd.set( true );
}


condition DummyHttpSrvNeg::cnd;

DummyHttpSrvNeg::DummyHttpSrvNeg( std::sockstream& )
{
}

void DummyHttpSrvNeg::connect( std::sockstream& s )
{
  BOOST_CHECK( s.good() );

  http::request rq;

  s >> noskipws >> http::body( true ) >> rq;

  // BOOST_CHECK( s.good() );

  if ( rq.head().value() == http::command::GET ) {
    http::response rs;
    rs.head().code( 410 );
    rs.body() = "1234567\n";

    (s << http::body( true ) << rs).flush();

    BOOST_CHECK( s.good() );
  } else if ( rq.head().value() == http::command::POST ) {
    http::response rs;
    rs.head().code( 410 );
    rs.body() = "1234567\n";

    (s << http::body( true ) << rs).flush();
    
    BOOST_CHECK( s.good() );
  }
}

void DummyHttpSrvNeg::close()
{
  cnd.set( true );
}

} // namespace test

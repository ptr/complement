#include "server.h"

#include <sockios/sockmgr.h>
#include <list>
#include <string>
#include <stdexcept>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "http.h"
#include "intercessor.h"

#include <stem/EventHandler.h>

extern boost::filesystem::path dir;
extern unsigned rq_timeout;

namespace intr {

using namespace std;
using namespace xmt;
using namespace stem;

class Notificator :
    public stem::EventHandler
{
  public:
    enum ReportWareRes
    {
      accepted = 0,
      rejected,
      unparsed,
      unknown
    };

    Notificator();

    void ok();
    void nok( const stem::Event& );
    void raw( const intr::httprs& );

    ReportWareRes wait();
    const http::response& response() const
      { return rs; }

  private:
    xmt::condition cnd;
    xmt::timespec t;
    ReportWareRes flag;
    http::response rs;


    DECLARE_RESPONSE_TABLE( Notificator, stem::EventHandler );
};

DEFINE_RESPONSE_TABLE( Notificator )
  EV_VOID(0,RESPONCE_OK,ok)
  EV_EDS(0,RESPONCE_NOK,nok)
  EV_T_( ST_NULL, RESPONCE_UNP, raw, intr::httprs )
END_RESPONSE_TABLE

Notificator::Notificator() :
    flag( unknown )
{
  cnd.set( false );
  xmt::gettime( &t );
  t.tv_sec += rq_timeout;
}

Notificator::ReportWareRes Notificator::wait()
{
  if ( cnd.try_wait_time( t ) ) {
    return unknown;
  }
  return flag;
}

void Notificator::ok()
{
  flag = accepted;
  cnd.set( true );
}

void Notificator::nok( const stem::Event& )
{
  flag = rejected;
  cnd.set( true );
}

void Notificator::raw( const intr::httprs& r )
{
  flag = unparsed;
  rs = r.rs;
  cnd.set( true );
}

IncomeHttpRqProcessor::IncomeHttpRqProcessor( std::sockstream& )
{
  // cerr << "ConnectionProcessor::ConnectionProcessor\n";
}

void IncomeHttpRqProcessor::connect( std::sockstream& s )
{
  // cerr << "ConnectionProcessor::connect\n";

  http::request rq;
  Notificator note;

  // read whole request (with body)
  s >> http::body( true ) >> rq;

  if ( rq.head().value() != http::command::GET && rq.head().value() != http::command::POST ) {
    http::response rs;
    rs.head().code( 405 );
    rs.headers().push_back( http::header( "Allow", "GET, POST" ) );
    (s << rs).flush();
    return;
  }

  if ( rq.search( "X-API-ReportFileName" ) == rq.headers().end() ) {
    http::response rs;
    rs.head().code( 412 );
    (s << rs).flush();
    return;    
  }

  if ( rq.search( "X-API-ReportFileName" )->value().find_first_not_of( "abcdefghijklmnopqrstuvwxyz0123456789._ABCDEFGHIJKLMNOPQRSTUVWXYZ" ) != string::npos ) {
    http::response rs;
    rs.head().code( 412 );
    (s << rs).flush();
    return;
  }

  namespace fs = boost::filesystem;

  if ( fs::exists( dir / rq.search( "X-API-ReportFileName" )->value() ) ) {
    http::response rs;
    rs.head().code( 412 );
    (s << rs).flush();
    return;    
  }

  // only GET or POST commands here

  stem::Event_base<intr::httprq> ev( INTR_RQ );
  ev.dest( 0 );
  ev.value().rq = rq;
  note.Send( ev );

  http::response rs;

  switch ( note.wait() ) {
    case Notificator::accepted:
      rs.head().code( 202 );
      rs.headers().push_back( http::header( "Content-Length", "0" ) );
      break;
    case Notificator::rejected:
      rs.head().code( 503 );
      rs.headers().push_back( http::header( "Content-Length", "0" ) );
      break;
    case Notificator::unknown: // ? hmm, 202 may be here
      rs.head().code( 504 );
      rs.headers().push_back( http::header( "Content-Length", "0" ) );
      break;
    case Notificator::unparsed: // keep response as is
      rs = note.response();
      break;
  }

  (s << rs).flush();

  if ( rq.head().value() == http::command::GET ) {
    s.rdbuf()->shutdown( sock_base::stop_in );
    // cerr << "'" << rq.body() << "'" << endl;
  }
}

void IncomeHttpRqProcessor::close()
{
  // cerr << "ConnectionProcessor::close\n";
}

} // namespace intr

// -*- C++ -*- Time-stamp: <06/09/12 17:19:46 ptr>

#include <string>
#include <iostream>
#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <stem/NetTransport.h>
#include <mt/xmt.h>

#include "stemdtch.h"

using namespace stem;
using namespace xmt;

class Sample :
    public stem::EventHandler
{
  public:
    Sample();
    Sample( stem::addr_type );
    ~Sample();

    void translate( const char *, size_t len );

  private:
    void echo( const stem::Event& );

    xmt::Condition cnd;
    stem::NetTransportMgr stemsrv;
    stem::addr_type stemaddr;

    DECLARE_RESPONSE_TABLE( Sample, stem::EventHandler );
};

Sample::Sample()
{
  cnd.set( false );
  stemaddr = stemsrv.open( "localhost", 6995 );
}

Sample::Sample( stem::addr_type id ) :
    stem::EventHandler( id )
{
  cnd.set( false );
  stemaddr = stemsrv.open( "localhost", 6995 );
}

void Sample::echo( const stem::Event& ev )
{
  cout << ev.value();
}

void Sample::translate( const char *m, size_t sz )
{
  if ( stemaddr != stem::badaddr ) {
    Event ev( 0x5000 );
    ev.dest( stemaddr );
    ev.value().assign( m, sz );

    Send( ev );
  } else {
    cerr << "Oh, srv address is bad!" << endl;
  }
}

DEFINE_RESPONSE_TABLE( Sample )
  EV_EDS( 0, 0x5000, echo )
END_RESPONSE_TABLE


Sample sample( 0 );

void send_msg( char *msg, unsigned msglen )
{
  sample.translate( msg, msglen );
}

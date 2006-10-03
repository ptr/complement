// -*- C++ -*- Time-stamp: <06/10/03 17:44:45 ptr>

#include <stem/EventHandler.h>
#include <stem/NetTransport.h>
#include <sockios/sockstream>
#include <sockios/sockmgr.h>
#include <string>
#include <mt/xmt.h>
#include <iostream>

using namespace stem;
using namespace xmt;
using namespace std;

class StEMecho :
    public stem::EventHandler
{
  public:
    StEMecho();
    StEMecho( stem::addr_type id );
    StEMecho( stem::addr_type id, const char * );

    void echo( const stem::Event& );

  private:
    DECLARE_RESPONSE_TABLE( StEMecho, stem::EventHandler );
};

StEMecho::StEMecho()
{
}

StEMecho::StEMecho( addr_type id ) :
    EventHandler( id )
{
}

StEMecho::StEMecho( addr_type id, const char *info ) :
    EventHandler( id, info )
{
}

void StEMecho::echo( const Event& ev )
{
  Event eev( 0x5000 );
  eev.value() = ev.value();

  eev.dest( ev.src() );

  Send( eev );

  cerr << ev.value() << ", " << hex << ev.src() << dec << endl;
}

DEFINE_RESPONSE_TABLE( StEMecho )
  EV_EDS( 0, 0x5000, echo )
END_RESPONSE_TABLE

int main()
{
  try {
    // xmt::Thread::become_daemon();

    StEMecho echo( 0, "echo service");
    sockmgr_stream_MP<NetTransport> srv( 6995 );

    srv.wait();
  }

  catch ( xmt::fork_in_parent& child ) {
    // child.pid();
  }
  catch ( std::runtime_error& err ) {
    cerr << err.what() << endl;
  }
  catch ( ... ) {
  }

  return 0;
}

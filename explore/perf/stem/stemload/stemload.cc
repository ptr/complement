// -*- C++ -*- Time-stamp: <06/09/14 17:46:32 ptr>

#include <stem/EventHandler.h>
#include <stem/NetTransport.h>
#include <sockios/sockstream>
#include <sockios/sockmgr.h>
#include <string>
#include <sstream>
#include <fstream>
#include <mt/xmt.h>

#include <cstdlib>

using namespace stem;
using namespace xmt;

class Simulator :
    public stem::EventHandler
{
  public:
    Simulator( int i );
    
    void echo( const stem::Event& );
    void loop();

  private:
    NetTransportMgr echosrv;
    addr_type echo_addr;
    ofstream timing;

    int n;

    DECLARE_RESPONSE_TABLE( Simulator, stem::EventHandler );
};

Simulator::Simulator( int i ) :
    n( i )
{
  echo_addr = echosrv.open( "localhost", 6995 );
  stringstream s;
  
  s << "stemload_log." << i;

  timing.open( s.str().c_str() );
}

void Simulator::loop()
{
  timespec tm;
  tm.tv_sec = 0;
  tm.tv_nsec = 300000000;

  timespec tm_mark;

  unsigned sand = 0;

  while ( true ) {
    Event ev( 0x5000 );
    ev.dest( echo_addr );
    
    Thread::gettime( &tm_mark );

    stringstream s;

    s << tm_mark.tv_sec << " " << tm_mark.tv_nsec << " ";

    double tmp = (double)rand_r( &sand ) / RAND_MAX * (1024.0 - 30.0) + 30.0 + 0.5;

    s << string( (string::size_type)tmp, ' ' ); // random mess size [30, 1024], + time mark

    ev.value() = s.str();

    Send( ev );

    tmp = (double)rand_r( &sand ) / RAND_MAX * 1.0e+9;

    tm.tv_nsec = (unsigned)tmp;

    Thread::delay( &tm ); // random delay: [0,1) sec
  }
}

void Simulator::echo( const stem::Event& ev )
{
  // cerr << "q";

  timespec tm_mark;
  Thread::gettime( &tm_mark );

  timespec tm_stored;
  stringstream s( ev.value() );

  s >> tm_stored.tv_sec >> tm_stored.tv_nsec;
  
  timespec tm_diff = tm_mark - tm_stored;

  timing << n << " " << ((double)tm_mark.tv_sec + tm_mark.tv_nsec / 1.0e+9) << " " << ((double)tm_diff.tv_sec + tm_diff.tv_nsec / 1.0e+9) << endl;
}

DEFINE_RESPONSE_TABLE( Simulator )
  EV_EDS( 0, 0x5000, echo )
END_RESPONSE_TABLE

Thread::ret_code client_thread( void *p )
{
  Simulator simulator( (int)p );

  simulator.loop();
}

int main()
{
  Condition cnd;

  cnd.set( false );
  timespec tm;
  tm.tv_sec = 0;
  tm.tv_nsec = 500000000;

  for ( int i = 0; i < 200; ++i ) {
    new Thread( client_thread, (void *)i );

    Thread::delay( &tm );
  }

  cnd.wait();

  return 0;
}

// -*- C++ -*- Time-stamp: <07/08/26 12:54:05 ptr>

#include <janus/vtime.h>
#include <janus/janus.h>
#include <janus/vshostmgr.h>

#include <stem/EvManager.h>

#include <mt/xmt.h>

#include <iostream>
#include <string>

using namespace janus;
using namespace std;
using namespace xmt;
using namespace stem;

#define VS_LINE 0x1300

class YaSample :
    public janus::VTHandler
{
  public:
    YaSample();
    YaSample( stem::addr_type id, const char *info = 0 );
    YaSample( const char *info );
    ~YaSample();

  private:
    void vs_line( const stem::Event& );

    virtual void VSNewMember( const stem::Event_base<VSsync_rq>& );
    virtual void VSOutMember( const stem::Event_base<VSsync_rq>& );
    virtual void VSsync_time( const stem::Event_base<VSsync>& );


    DECLARE_RESPONSE_TABLE( YaSample, janus::VTHandler );
};

YaSample::YaSample() :
    janus::VTHandler()
{
}

YaSample::YaSample( stem::addr_type id, const char *info ) :
    janus::VTHandler( id, info )
{
}

YaSample::YaSample( const char *info ) :
    janus::VTHandler( info )
{
}

YaSample::~YaSample()
{
}

void YaSample::VSNewMember( const stem::Event_base<VSsync_rq>& ev )
{
  // VTNewMember_data( ev, "" );
  // cerr << "new member" << endl;
  VTHandler::VSNewMember( ev );
}

void YaSample::VSOutMember( const stem::Event_base<VSsync_rq>& )
{
  // cerr << "member out" << endl;
}

void YaSample::VSsync_time( const stem::Event_base<VSsync>& ev )
{
  // cout << "VSsync_time" << endl;
  VTHandler::VSsync_time( ev );
}

void YaSample::vs_line( const stem::Event& ev )
{
  // cerr << "Line here: '" << ev.value() << "'" << endl;
}

DEFINE_RESPONSE_TABLE( YaSample )
  EV_EDS( ST_NULL, VS_LINE, vs_line )
END_RESPONSE_TABLE

int main()
{
  VSHostMgr::add_srvport( 6700 );
  VSHostMgr::add_wellknown( "island.corbina.net:6700" );

  YaSample sample;

  // sample.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch | stem::EvManager::tracefault );
  // sample.manager()->settrs( &std::cerr );

  // sample.vtdispatcher()->settrf( janus::Janus::tracenet | janus::Janus::tracedispatch | janus::Janus::tracefault | janus::Janus::tracedelayed | janus::Janus::tracegroup );
  // sample.vtdispatcher()->settrs( &std::cerr );

  sample.JoinGroup( janus::vs_base::first_user_group );

  for ( int i = 0; i < 50000; ++i ) {
    YaSample *s = new YaSample;
    s->JoinGroup( janus::vs_base::first_user_group + i + 1 );
  }

  cout << "ready" << endl;

  Event ev( VS_LINE );
  ev.dest( janus::vs_base::first_user_group );

  string line;

  while ( cin.good() ) {
    getline( cin, line );
    if ( !cin.fail() ) {
      cerr << "local: " << line << endl;
      ev.value() = line;
      try {
        sample.JaSend( ev );
      }
      catch ( std::domain_error& err ) {
        cerr << err.what() << endl;
      }
    }
  }

  // condition cnd;

  // cnd.set( false );

  // cnd.wait();  

  return 0;
}

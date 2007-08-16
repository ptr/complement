// -*- C++ -*- Time-stamp: <07/08/16 10:45:48 ptr>

#include "vt_operations.h"

// #include <boost/lexical_cast.hpp>

#include <iostream>
#include <janus/vtime.h>

#include <stem/EvManager.h>
#include <stem/NetTransport.h>
#include <sockios/sockmgr.h>
#include <sys/wait.h>

#include <mt/xmt.h>
#include <mt/shm.h>

using namespace std;
using namespace stem;
using namespace xmt;
using namespace vt;

class YaRemote :
    public vt::VTHandler
{
  public:
    YaRemote();
    YaRemote( stem::addr_type id );
    YaRemote( stem::addr_type id, const char *info );
    ~YaRemote();

    void handler( const stem::Event& );
    void VSNewMember( const stem::Event_base<VSsync_rq>& );
    void VSOutMember( const stem::Event_base<VSsync_rq>& );

    void greeting();

    void wait();
    std::string msg;
    int count;
    int ocount;

    void wait_greeting()
      {
        gr.try_wait();
        gr.set( false );
      }

  private:
    xmt::condition cnd;
    xmt::condition gr;

    DECLARE_RESPONSE_TABLE( YaRemote, vt::VTHandler );
};

#define VS_DUMMY_MESS     0x1203
#define VS_DUMMY_GREETING 0x1204

YaRemote::YaRemote() :
    VTHandler(),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );

  JoinGroup( 0 );
}

YaRemote::YaRemote( stem::addr_type id ) :
    VTHandler( id ),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );

  JoinGroup( 0 );
}

YaRemote::YaRemote( stem::addr_type id, const char *info ) :
    VTHandler( id, info ),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );

  JoinGroup( 0 );
}

YaRemote::~YaRemote()
{
  // cnd.wait();
}

void YaRemote::handler( const stem::Event& ev )
{
  msg = ev.value();

  cnd.set( true );
}

void YaRemote::VSNewMember( const stem::Event_base<VSsync_rq>& ev )
{
  // cerr << "Hello " << ev.src() << endl;
  ++count;

  // VTNewMember_data( ev, "" );
  VTHandler::VSNewMember( ev );

  stem::EventVoid gr_ev( VS_DUMMY_GREETING );
  gr_ev.dest( ev.src() );
  Send( gr_ev );
}

void YaRemote::VSOutMember( const stem::Event_base<VSsync_rq>& ev )
{
  // cerr << "Hello" << endl;
  ++ocount;
}

void YaRemote::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

void YaRemote::greeting()
{
  gr.set( true );
}

DEFINE_RESPONSE_TABLE( YaRemote )
  EV_EDS( ST_NULL, VS_DUMMY_MESS, handler )
  EV_VOID( ST_NULL, VS_DUMMY_GREETING, greeting )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::remote)
{
  const char fname[] = "/tmp/yanus_test.shm";
  xmt::shm_alloc<0> seg;
  xmt::allocator_shm<xmt::__condition<true>,0> shm_cnd;
  xmt::allocator_shm<xmt::__barrier<true>,0>   shm_b;

  try {
    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
    xmt::__barrier<true>& b = *new ( shm_b.allocate( 1 ) ) xmt::__barrier<true>();

    try {
      xmt::fork();

      b.wait();

      NetTransportMgr mgr;

      addr_type zero = mgr.open( "localhost", 6980 );

      EXAM_CHECK_ASYNC( mgr.good() );

      YaRemote obj2;

      exit(0);
    }
    catch ( xmt::fork_in_parent& child ) {
      sockmgr_stream_MP<NetTransport> srv( 6980 );

      EXAM_REQUIRE( srv.good() );

      b.wait();

      YaRemote obj1;

      int stat;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );

      srv.close();
      srv.wait();
    }

    (&b)->~__barrier<true>();
    shm_b.deallocate( &b, 1 );
  }
  catch ( const xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  seg.deallocate();
  unlink( fname );

  return EXAM_RESULT;
}


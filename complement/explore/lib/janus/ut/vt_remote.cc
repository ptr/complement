// -*- C++ -*- Time-stamp: <07/08/23 12:43:15 ptr>

#include "vt_operations.h"

#include <iostream>
#include <janus/vtime.h>
#include <janus/janus.h>
#include <janus/vshostmgr.h>

#include <stem/EvManager.h>
#include <stem/NetTransport.h>
#include <stem/Event.h>
#include <stem/EvPack.h>
#include <sockios/sockmgr.h>
#include <sys/wait.h>

#include <mt/xmt.h>
#include <mt/shm.h>
#include <mt/time.h>

#include <list>
#include <sstream>

using namespace std;
using namespace stem;
using namespace xmt;
using namespace janus;

#define VS_DUMMY_MESS     0x1203
#define VS_DUMMY_GREETING 0x1204

class YaRemote :
    public janus::VTHandler
{
  public:
    YaRemote();
    YaRemote( stem::addr_type id, const char *info = 0 );
    YaRemote( const char *info );
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

    DECLARE_RESPONSE_TABLE( YaRemote, janus::VTHandler );
};

YaRemote::YaRemote() :
    VTHandler(),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );
}

YaRemote::YaRemote( stem::addr_type id, const char *info ) :
    VTHandler( id, info ),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );
}

YaRemote::YaRemote( const char *info ) :
    VTHandler( info ),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );
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
  // cerr << "Hello " << xmt::getpid() << endl;
  ++count;

  // VTNewMember_data( ev, "" );
  VTHandler::VSNewMember( ev );

  stem::EventVoid gr_ev( VS_DUMMY_GREETING );
  gr_ev.dest( ev.src() );
  Send( gr_ev );
}

void YaRemote::VSOutMember( const stem::Event_base<VSsync_rq>& ev )
{
  cerr << "VSOutMember" << endl;
  ++ocount;
}

void YaRemote::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

void YaRemote::greeting()
{
  if ( count > 0 ) {
    gr.set( true );
  }
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

      {
        YaRemote obj1( "obj client" );

        // obj1.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch );
        // obj1.manager()->settrs( &std::cerr );

        // obj1.vtdispatcher()->settrf( janus::Janus::tracenet | janus::Janus::tracedispatch | janus::Janus::tracefault | janus::Janus::tracedelayed | janus::Janus::tracegroup );
        // obj1.vtdispatcher()->settrs( &std::cerr );

        EXAM_CHECK_ASYNC( obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) == 1 );

        obj1.vtdispatcher()->connect( "localhost", 6980 );

        // cerr << obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) << endl;

        while ( obj1.vtdispatcher()->vs_known_processes() < 2 ) {
          xmt::Thread::yield();
          xmt::delay( xmt::timespec( 0, 1000000 ) );
        }

        /* ******************************************************************************
           This variant is wrong, because of group_size don't guarantee that information
           in the object is relevant (i.e. VSsync happens); for example, in case below
           group_size already 2, but no janus string stored yet.

        while ( obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) < 2 ) {
          xmt::Thread::yield();
          xmt::delay( xmt::timespec( 0, 1000000 ) );
        }
         * ****************************************************************************** */

        // cerr << obj1.vtdispatcher()->vs_known_processes() << endl;

        EXAM_CHECK_ASYNC( obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) == 2 );
        // cerr << obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) << endl;

        obj1.JoinGroup( janus::vs_base::first_user_group );

        obj1.wait_greeting();

        EXAM_CHECK_ASYNC( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 2 );

        // cerr << "* " << obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) << endl;
        obj1.wait();
      }

      exit(0);
    }
    catch ( xmt::fork_in_parent& child ) {
      YaRemote obj1( "obj srv" );

      // obj1.vtdispatcher()->settrf( janus::Janus::tracenet | janus::Janus::tracedispatch | janus::Janus::tracefault | janus::Janus::tracedelayed | janus::Janus::tracegroup );
      // obj1.vtdispatcher()->settrs( &std::cerr );

      obj1.vtdispatcher()->serve( 6980 );

      obj1.JoinGroup( janus::vs_base::first_user_group );

      b.wait();

      // while ( obj1.vtdispatcher()->vs_known_processes() < 2 ) {
      //   xmt::delay( xmt::timespec( 0, 1000000 ) );
      // }

      obj1.wait_greeting();

      stem::Event ev( VS_DUMMY_MESS );
      ev.dest( janus::vs_base::first_user_group );
      ev.value() = "hello";

      obj1.JaSend( ev );

      int stat;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );

      // cerr << obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) << endl;
      // cerr << obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) << endl;
      // cerr << obj1.vtdispatcher()->vs_known_processes() << endl;
      // cerr << obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) << endl;
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


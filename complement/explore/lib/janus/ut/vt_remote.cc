// -*- C++ -*- Time-stamp: <07/08/26 12:54:05 ptr>

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

namespace janus {

using namespace std;
using namespace stem;
using namespace xmt;

#define VS_DUMMY_MESS       0x1203
#define VS_DUMMY_GREETING   0x1204
#define VS_DUMMY_GREETING2  0x1205
#define VS_DUMMY_MESS2      0x1206

class YaRemote :
    public janus::VTHandler
{
  public:
    YaRemote();
    YaRemote( stem::addr_type id, const char *info = 0 );
    YaRemote( const char *info );
    ~YaRemote();

    void handler( const stem::Event& );
    void handler2( const stem::Event& );
    void VSNewMember( const stem::Event_base<VSsync_rq>& );
    void VSOutMember( const stem::Event_base<VSsync_rq>& );

    void greeting();
    void greeting2();

    void wait();
    void wait2();

    std::string msg;
    int count;
    int ocount;

    void wait_greeting()
      {
        gr.try_wait();
        gr.set( false );
      }

    void wait_greeting2()
      {
        gr2.try_wait();
        gr2.set( false );
      }

  private:
    xmt::condition cnd;
    xmt::condition cnd2;
    xmt::condition gr;
    xmt::condition gr2;

    DECLARE_RESPONSE_TABLE( YaRemote, janus::VTHandler );
};

YaRemote::YaRemote() :
    VTHandler(),
    count(0),
    ocount(0)
{
  cnd.set( false );
  cnd2.set( false );
  gr.set( false );
  gr2.set( false );
}

YaRemote::YaRemote( stem::addr_type id, const char *info ) :
    VTHandler( id, info ),
    count(0),
    ocount(0)
{
  cnd.set( false );
  cnd2.set( false );
  gr.set( false );
  gr2.set( false );
}

YaRemote::YaRemote( const char *info ) :
    VTHandler( info ),
    count(0),
    ocount(0)
{
  cnd.set( false );
  cnd2.set( false );
  gr.set( false );
  gr2.set( false );
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

void YaRemote::handler2( const stem::Event& ev )
{
  msg = ev.value();

  cnd2.set( true );
}

void YaRemote::VSNewMember( const stem::Event_base<VSsync_rq>& ev )
{
  // cerr << "Hello " << xmt::getpid() << endl;
  ++count;

  // VTNewMember_data( ev, "" );
  VTHandler::VSNewMember( ev );

  if ( ev.value().grp == janus::vs_base::first_user_group ) {
    stem::EventVoid gr_ev( VS_DUMMY_GREETING );
    gr_ev.dest( ev.src() );
    Send( gr_ev );
  } else if ( ev.value().grp == (janus::vs_base::first_user_group + 1) ) {
    stem::EventVoid gr_ev( VS_DUMMY_GREETING2 );
    gr_ev.dest( ev.src() );
    Send( gr_ev );
  }
}

void YaRemote::VSOutMember( const stem::Event_base<VSsync_rq>& ev )
{
  ++ocount;
}

void YaRemote::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

void YaRemote::wait2()
{
  cnd2.try_wait();

  cnd2.set( false );
}

void YaRemote::greeting()
{
  gr.set( true );
}

void YaRemote::greeting2()
{
  gr2.set( true );
}

DEFINE_RESPONSE_TABLE( YaRemote )
  EV_EDS( ST_NULL, VS_DUMMY_MESS, handler )
  EV_EDS( ST_NULL, VS_DUMMY_MESS2, handler2 )
  EV_VOID( ST_NULL, VS_DUMMY_GREETING, greeting )
  EV_VOID( ST_NULL, VS_DUMMY_GREETING2, greeting2 )
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

      long res_flag = 0;

      b.wait();

      {
        VTHandler obj0; // this need to keep VSHostMgr after YaRemote exit
                        // to check exit from group with another process
        {
          YaRemote obj1( "obj client" );

          // obj1.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch );
          // obj1.manager()->settrs( &std::cerr );

          // obj1.vtdispatcher()->settrf( janus::Janus::tracenet | janus::Janus::tracedispatch | janus::Janus::tracefault | janus::Janus::tracedelayed | janus::Janus::tracegroup );
          // obj1.vtdispatcher()->settrs( &std::cerr );

          EXAM_CHECK_ASYNC_F( obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) == 1, res_flag );

          obj1.vtdispatcher()->connect( "localhost", 6980 );

          // cerr << obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) << endl;

          while ( obj1.vtdispatcher()->vs_known_processes() < 2 ) {
            xmt::Thread::yield();
            xmt::delay( xmt::timespec( 0, 1000000 ) );
          }

          /*******************************************************************\
           * This variant is wrong, because of group_size don't guarantee
           * that information in the object is relevant (i.e. VSsync happens);
           * for example, in case below group_size already 2, but no janus string
           * stored yet.

          while ( obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) < 2 ) {
            xmt::Thread::yield();
            xmt::delay( xmt::timespec( 0, 1000000 ) );
          }
          \********************************************************************/

          // cerr << obj1.vtdispatcher()->vs_known_processes() << endl;

          EXAM_CHECK_ASYNC_F( obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) == 2, res_flag );
          // cerr << obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) << endl;

          obj1.JoinGroup( janus::vs_base::first_user_group );

          obj1.wait_greeting();

          EXAM_CHECK_ASYNC_F( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 2, res_flag );
          EXAM_CHECK_ASYNC_F( obj1.count == 1, res_flag );
          // cerr << "* " << obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) << endl;
          obj1.wait();

          // obj1.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch );
          // obj1.manager()->settrs( &std::cerr );
        }
        // obj1 here away, but in another process (remote) still exist object in
        // first_user_group, that's why 1 here:
        EXAM_CHECK_ASYNC_F( obj0.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 1, res_flag );
      }

      exit( res_flag );
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

      EXAM_CHECK( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 2 );
      EXAM_CHECK( obj1.count == 1 );

      // obj1.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch );
      // obj1.manager()->settrs( &std::cerr );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }

      EXAM_CHECK( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 1 );

      EXAM_CHECK( obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) == 1 );
      // cerr << obj1.vtdispatcher()->vs_known_processes() << endl;
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

int EXAM_IMPL(vtime_operations::mgroups)
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

      long res_flag = 0;

      b.wait();

      {
        VTHandler obj0; // this need to keep VSHostMgr after YaRemote exit
                        // to check exit from group with another process
        {
          YaRemote obj1( "obj client" );

          obj1.vtdispatcher()->connect( "localhost", 6980 );

          while ( obj1.vtdispatcher()->vs_known_processes() < 2 ) {
            xmt::Thread::yield();
            xmt::delay( xmt::timespec( 0, 1000000 ) );
          }

          obj1.JoinGroup( janus::vs_base::first_user_group );
          obj1.JoinGroup( janus::vs_base::first_user_group + 1);

          obj1.wait_greeting();
          obj1.wait_greeting2();

          EXAM_CHECK_ASYNC_F( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 2, res_flag );
          EXAM_CHECK_ASYNC_F( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group + 1) == 2, res_flag );
          EXAM_CHECK_ASYNC_F( obj1.count == 2, res_flag );

          stem::Event ev( VS_DUMMY_MESS2 );
          ev.dest( janus::vs_base::first_user_group + 1 );
          ev.value() = "hello";

          obj1.JaSend( ev );

          obj1.wait();

          // obj1.manager()->settrf( stem::EvManager::tracenet | stem::EvManager::tracedispatch );
          // obj1.manager()->settrs( &std::cerr );
        }

        EXAM_CHECK_ASYNC_F( obj0.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 1, res_flag );
        EXAM_CHECK_ASYNC_F( obj0.vtdispatcher()->group_size(janus::vs_base::first_user_group + 1) == 1, res_flag );
      }

      exit( res_flag );
    }
    catch ( xmt::fork_in_parent& child ) {
      YaRemote obj1( "obj srv" );

      obj1.vtdispatcher()->serve( 6980 );

      obj1.JoinGroup( janus::vs_base::first_user_group );
      obj1.JoinGroup( janus::vs_base::first_user_group + 1);

      b.wait();

      obj1.wait_greeting();
      obj1.wait_greeting2();

      stem::Event ev( VS_DUMMY_MESS );
      ev.dest( janus::vs_base::first_user_group );
      ev.value() = "hello";

      obj1.JaSend( ev );

      EXAM_CHECK( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 2 );
      EXAM_CHECK( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group + 1) == 2 );
      EXAM_CHECK( obj1.count == 2  );

      obj1.wait2();

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }

      EXAM_CHECK( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group) == 1 );
      EXAM_CHECK( obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group + 1) == 1 );
      // cerr << obj1.vtdispatcher()->group_size(janus::vs_base::first_user_group + 1) << endl;

      EXAM_CHECK( obj1.vtdispatcher()->group_size(janus::vs_base::vshosts_group) == 1 );
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

} // namespace janus

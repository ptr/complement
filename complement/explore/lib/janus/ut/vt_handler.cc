// -*- C++ -*- Time-stamp: <07/08/17 10:41:22 ptr>

#include "vt_operations.h"

#include <iostream>
#include <janus/vtime.h>

#include <stem/EvManager.h>

using namespace janus;
using namespace std;

class VTDummy :
    public janus::VTHandler
{
  public:
    VTDummy();
    VTDummy( stem::addr_type id );
    VTDummy( stem::addr_type id, const char *info );
    ~VTDummy();

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

    DECLARE_RESPONSE_TABLE( VTDummy, janus::VTHandler );
};

#define VS_DUMMY_MESS     0x1203
#define VS_DUMMY_GREETING 0x1204

VTDummy::VTDummy() :
    VTHandler(),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );

  JoinGroup( first_user_group );
}

VTDummy::VTDummy( stem::addr_type id ) :
    VTHandler( id ),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );

  JoinGroup( first_user_group );
}

VTDummy::VTDummy( stem::addr_type id, const char *info ) :
    VTHandler( id, info ),
    count(0),
    ocount(0)
{
  cnd.set( false );
  gr.set( false );

  JoinGroup( first_user_group );
}

VTDummy::~VTDummy()
{
  // cnd.wait();
}

void VTDummy::handler( const stem::Event& ev )
{
  msg = ev.value();

  cnd.set( true );
}

void VTDummy::VSNewMember( const stem::Event_base<VSsync_rq>& ev )
{
  // cerr << "Hello " << ev.src() << endl;
  ++count;

  // VTNewMember_data( ev, "" );
  VTHandler::VSNewMember( ev );

  stem::EventVoid gr_ev( VS_DUMMY_GREETING );
  gr_ev.dest( ev.src() );
  Send( gr_ev );
}

void VTDummy::VSOutMember( const stem::Event_base<VSsync_rq>& ev )
{
  // cerr << "Hello" << endl;
  ++ocount;
}

void VTDummy::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

void VTDummy::greeting()
{
  gr.set( true );
}

DEFINE_RESPONSE_TABLE( VTDummy )
  EV_EDS( ST_NULL, VS_DUMMY_MESS, handler )
  EV_VOID( ST_NULL, VS_DUMMY_GREETING, greeting )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VTHandler1)
{
  VTDummy dummy1;
  VTDummy dummy2;

  stem::Event ev( VS_DUMMY_MESS );
  ev.dest( janus::vs_base::first_user_group ); // group
  ev.value() = "hello";

  dummy1.JaSend( ev );

  dummy2.wait();

  EXAM_CHECK( dummy2.msg == "hello" );
  EXAM_CHECK( dummy1.msg == "" );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VTHandler2)
{
  VTDummy dummy1;
  VTDummy dummy2;
  VTDummy dummy3;

  stem::Event ev( VS_DUMMY_MESS );
  ev.dest( janus::vs_base::first_user_group ); // group
  ev.value() = "hello";

  dummy1.JaSend( ev );

  dummy2.wait();
  dummy3.wait();

  EXAM_CHECK( dummy3.count == 0 );
  EXAM_CHECK( dummy3.msg == "hello" );
  EXAM_CHECK( dummy2.count == 1 );
  EXAM_CHECK( dummy2.msg == "hello" );
  EXAM_CHECK( dummy1.count == 2 );
  EXAM_CHECK( dummy1.msg == "" );

  ev.dest( janus::vs_base::first_user_group + 100 ); // not this group member
  try {
    dummy1.JaSend( ev );
    EXAM_ERROR( "exception expected" );
  }
  catch ( std::domain_error& ) {
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VTSubscription)
{
  VTDummy dummy1;
  VTDummy dummy2;

  stem::Event ev( VS_DUMMY_MESS );
  ev.dest( janus::vs_base::first_user_group ); // group
  ev.value() = "hello";

  dummy1.JaSend( ev );

  dummy2.wait();
  EXAM_CHECK( dummy2.msg == "hello" );

  {
    VTDummy dummy3;

    ev.value() = "hi";
    dummy1.JaSend( ev );

    dummy2.wait();
    // dummy3.wait();

    // EXAM_CHECK( dummy3.msg == "hi" );
    // EXAM_CHECK( dummy3.msg == "" ); // dummy3 don't see, due to VTS
    EXAM_CHECK( dummy2.msg == "hi" );
    EXAM_CHECK( dummy1.msg == "" );
  }

  ev.value() = "yet more";
  dummy1.JaSend( ev );

  dummy2.wait();
  EXAM_CHECK( dummy2.msg == "yet more" );
  EXAM_CHECK( dummy1.msg == "" );

  EXAM_CHECK( dummy1.ocount == 1 );
  EXAM_CHECK( dummy2.ocount == 1 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VTEntryIntoGroup)
{
  VTDummy dummy1;

  stem::Event ev( VS_DUMMY_MESS );
  ev.dest( janus::vs_base::first_user_group ); // group
  ev.value() = "hello";

  {
    // dummy1.manager()->settrf( /* stem::EvManager::tracenet | */ stem::EvManager::tracedispatch );
    // dummy1.manager()->settrs( &std::cerr );

    VTDummy dummy3;

    dummy3.wait_greeting();

    ev.value() = "hi";
    dummy1.JaSend( ev );

    dummy3.wait();

    EXAM_CHECK( dummy3.msg == "hi" );
    EXAM_CHECK( dummy1.msg == "" );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VTEntryIntoGroup2)
{
  VTDummy dummy1;
  VTDummy dummy2;

  stem::Event ev( VS_DUMMY_MESS );
  ev.dest( janus::vs_base::first_user_group ); // group
  ev.value() = "hello";

  dummy1.JaSend( ev );

  dummy2.wait();
  EXAM_CHECK( dummy2.msg == "hello" );

  {
    // cerr << (void *)&dummy1 << " dummy1\n"
    //      << (void *)&dummy2 << " dummy2\n";
    // dummy1.manager()->settrf( /* stem::EvManager::tracenet | */ stem::EvManager::tracedispatch | stem::EvManager::tracefault );
    // dummy1.manager()->settrs( &std::cerr );

    // dummy1.vtdispatcher()->settrf( VTDispatcher::tracedispatch | VTDispatcher::tracefault | VTDispatcher::tracedelayed | VTDispatcher::tracegroup );
    // dummy1.vtdispatcher()->settrs( &std::cerr );

    VTDummy dummy3;

    dummy3.wait_greeting();

    ev.value() = "hi";
    ev.dest( janus::vs_base::first_user_group ); // group
    dummy1.JaSend( ev );

    dummy2.wait();
    dummy3.wait();

    EXAM_CHECK( dummy2.msg == "hi" );
    EXAM_CHECK( dummy3.msg == "hi" );
    EXAM_CHECK( dummy1.msg == "" );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VTEntryIntoGroup3)
{
  VTDummy dummy1;

  stem::Event ev( VS_DUMMY_MESS );

  {
    VTDummy dummy2;

    dummy2.wait_greeting();

    ev.value() = "hello";
    ev.dest( janus::vs_base::first_user_group ); // group
    dummy1.JaSend( ev );

    dummy2.wait();
    EXAM_CHECK( dummy2.msg == "hello" );
    EXAM_CHECK( dummy1.msg == "" );
  }

  {
    VTDummy dummy3;

    dummy3.wait_greeting();

    ev.value() = "hi";
    ev.dest( janus::vs_base::first_user_group ); // group
    dummy1.JaSend( ev );

    dummy3.wait();

    EXAM_CHECK( dummy3.msg == "hi" );
    EXAM_CHECK( dummy1.msg == "" );
  }

  {
    VTDummy dummy2;
    VTDummy dummy3;

    dummy2.wait_greeting();
    dummy3.wait_greeting();

    ev.value() = "more";
    ev.dest( janus::vs_base::first_user_group ); // group
    dummy1.JaSend( ev );

    dummy2.wait();
    dummy3.wait();

    EXAM_CHECK( dummy2.msg == "more" );
    EXAM_CHECK( dummy3.msg == "more" );
    EXAM_CHECK( dummy1.msg == "" );

    dummy2.JaSend( ev );

    dummy1.wait();
  }

  EXAM_CHECK( dummy1.msg == "more" );

  return EXAM_RESULT;
}

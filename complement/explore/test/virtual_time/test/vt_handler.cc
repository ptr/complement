// -*- C++ -*- Time-stamp: <07/07/26 09:53:24 ptr>

#include "vt_operations.h"

// #include <boost/lexical_cast.hpp>

#include <iostream>
#include <vtime.h>

using namespace vt;
using namespace std;

class VTDummy :
    public vt::VTHandler
{
  public:
    VTDummy();
    VTDummy( stem::addr_type id );
    VTDummy( stem::addr_type id, const char *info );
    ~VTDummy();

    void handler( const stem::Event& );
    void VTNewMember( const stem::Event& );

    void wait();
    std::string msg;
    int count;

  private:
    xmt::condition cnd;

    DECLARE_RESPONSE_TABLE( VTDummy, vt::VTHandler );
};

#define VT_MESS3  0x1203

VTDummy::VTDummy() :
    VTHandler(),
    count(0)
{
  cnd.set( false );
}

VTDummy::VTDummy( stem::addr_type id ) :
    VTHandler( id ),
    count(0)
{
  cnd.set( false );
}

VTDummy::VTDummy( stem::addr_type id, const char *info ) :
    VTHandler( id, info ),
    count(0)
{
  cnd.set( false );
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

void VTDummy::VTNewMember( const stem::Event& ev )
{
  // cerr << "Hello" << endl;
  ++count;
}

void VTDummy::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

DEFINE_RESPONSE_TABLE( VTDummy )
  EV_EDS( ST_NULL, VT_MESS3, handler )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VTHandler1)
{
  VTDummy dummy1;
  VTDummy dummy2;

  stem::Event ev( VT_MESS3 );
  ev.dest( 0 ); // group
  ev.value() = "hello";

  dummy1.VTSend( ev );

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

  stem::Event ev( VT_MESS3 );
  ev.dest( 0 ); // group
  ev.value() = "hello";

  dummy1.VTSend( ev );

  dummy2.wait();
  dummy3.wait();

  EXAM_CHECK( dummy3.count == 0 );
  EXAM_CHECK( dummy3.msg == "hello" );
  EXAM_CHECK( dummy2.count == 1 );
  EXAM_CHECK( dummy2.msg == "hello" );
  EXAM_CHECK( dummy1.count == 2 );
  EXAM_CHECK( dummy1.msg == "" );

  ev.dest( 100 ); // not this group member
  try {
    dummy1.VTSend( ev );
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

  stem::Event ev( VT_MESS3 );
  ev.dest( 0 ); // group
  ev.value() = "hello";

  dummy1.VTSend( ev );

  dummy2.wait();
  EXAM_CHECK( dummy2.msg == "hello" );

  {
    VTDummy dummy3;

    ev.value() = "hi";
    dummy1.VTSend( ev );

    dummy2.wait();
    // dummy3.wait();

    // EXAM_CHECK( dummy3.msg == "hi" );
    EXAM_CHECK( dummy3.msg == "" ); // dummy3 don't see, due to VTS
    EXAM_CHECK( dummy2.msg == "hi" );
    EXAM_CHECK( dummy1.msg == "" );
  }

  ev.value() = "yet more";
  dummy1.VTSend( ev );

  dummy2.wait();
  EXAM_CHECK( dummy2.msg == "yet more" );
  EXAM_CHECK( dummy1.msg == "" );

  return EXAM_RESULT;
}


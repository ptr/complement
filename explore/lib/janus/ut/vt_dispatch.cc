// -*- C++ -*- Time-stamp: <07/08/21 11:09:27 ptr>

#include "vt_operations.h"

#include <iostream>

#include <janus/vtime.h>
#include <janus/janus.h>

namespace janus {

using namespace std;

class Dummy :
    public stem::EventHandler
{
  public:
    Dummy();
    Dummy( stem::addr_type id );
    Dummy( stem::addr_type id, const char *info );
    ~Dummy();

    void handler( const stem::Event& );

    void wait();
    std::string msg;

  private:
    xmt::condition cnd;

    DECLARE_RESPONSE_TABLE( Dummy, stem::EventHandler );
};

#define VT_MESS2  0x1202

Dummy::Dummy() :
    EventHandler()
{
  cnd.set( false );
}

Dummy::Dummy( stem::addr_type id ) :
    EventHandler( id )
{
  cnd.set( false );
}

Dummy::Dummy( stem::addr_type id, const char *info ) :
    EventHandler( id, info )
{
  cnd.set( false );
}

Dummy::~Dummy()
{
  // cnd.wait();
}

void Dummy::handler( const stem::Event& ev )
{
  msg = ev.value();

  cnd.set( true );
}

void Dummy::wait()
{
  cnd.try_wait();

  cnd.set( false );
}

DEFINE_RESPONSE_TABLE( Dummy )
  EV_EDS( ST_NULL, VT_MESS2, handler )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VTDispatch1)
{
  janus::Janus dsp;
  Dummy dummy1;
  Dummy dummy2;
  const oid_type t1(1);
  const oid_type t2(2);

  dsp.Subscribe( dummy1.self_id(), t1, janus::vs_base::first_user_group );
  dsp.Subscribe( dummy2.self_id(), t2, janus::vs_base::first_user_group );

  stem::Event ev( VT_MESS2 );
  ev.src( dummy1.self_id() );

  ev.value() = "hello";

  dsp.JaSend( ev, janus::vs_base::first_user_group );

  dummy2.wait();

  EXAM_CHECK( dummy2.msg == "hello" );
  EXAM_CHECK( dummy1.msg == "" );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VTDispatch2)
{
  janus::Janus dsp;
  Dummy dummy1;
  Dummy dummy2;
  Dummy dummy3;
  const oid_type t1(1);
  const oid_type t2(2);
  const oid_type t3(3);

  dsp.Subscribe( dummy1.self_id(), t1, janus::vs_base::first_user_group );
  dsp.Subscribe( dummy2.self_id(), t2, janus::vs_base::first_user_group );
  dsp.Subscribe( dummy3.self_id(), t3, janus::vs_base::first_user_group );

  stem::Event ev( VT_MESS2 );
  ev.src( dummy1.self_id() );

  ev.value() = "hello";

  dsp.JaSend( ev, janus::vs_base::first_user_group );

  dummy2.wait();
  dummy3.wait();

  EXAM_CHECK( dummy3.msg == "hello" );
  EXAM_CHECK( dummy2.msg == "hello" );
  EXAM_CHECK( dummy1.msg == "" );

  return EXAM_RESULT;
}

} // namespace janus


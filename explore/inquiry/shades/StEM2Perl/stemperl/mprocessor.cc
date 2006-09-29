#include "mprocessor.h"

namespace test {

using namespace stem;
using namespace std;
using namespace xmt;

MProcessor::MProcessor( const char *srv_name ) :
    EventHandler( srv_name )
{
}

void MProcessor::send( stem::addr_type addr, const char *msg )
{
  Event ev( /* 0x7200 */ 0x5000 );

  ev.dest( addr );
  ev.value() = msg;

  Send( ev );
}

void MProcessor::send( stem::addr_type addr, const char *msg, size_t len )
{
  Event ev( /* 0x7200 */ 0x5000 );

  ev.dest( addr );
  ev.value().assign( msg, len );

  Send( ev );
}

void MProcessor::receive( const stem::Event& ev )
{
  MT_REENTRANT( lock, _1 );

  income_queue.push_back( ev );
}

const char *MProcessor::get()
{
  MT_REENTRANT( lock, _1 );
  if ( !income_queue.empty() ) {
    last = income_queue.front();
    income_queue.pop_front();

    return last.value().c_str();
  }
  return 0;
}

DEFINE_RESPONSE_TABLE( MProcessor )
  EV_EDS(0, /* 0x7200 */ 0x5000, receive)
END_RESPONSE_TABLE

}

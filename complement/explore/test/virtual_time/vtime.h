// -*- C++ -*- Time-stamp: <07/03/06 19:23:37 ptr>

#ifndef __vtime_h
#define __vtime_h

#include <stem/Event.h>
#include <stem/EventHandler.h>

namespace vt {

class Proc
  public stem::EventHandler
{
  public:
    Proc()
      { }
    Proc( stem::addr_type id ) :
        stem::EventHandler( id )
      { }

    void mess( const stem::Event_base<>& );

  private:
    DECLARE_RESPONSE_TABLE( Proc, stem::EventHandler );
};

#define MESS 0x300

} // namespace vt

#endif // __vtime_h

// -*- C++ -*- Time-stamp: <07/03/07 15:53:23 ptr>

#include "vtime.h"

namespace vt {

void Proc::mess( const stem::Event_base<>& ev )
{
}

DEFINE_RESPONSE_TABLE( Proc )
  EV_Event_base_T_( ST_NULL, MESS, mess, intr::httprq )
END_RESPONSE_TABLE

} // namespace vt


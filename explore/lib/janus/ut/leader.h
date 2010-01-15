// -*- C++ -*- Time-stamp: <10/01/15 21:05:34 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __leader_h
#define __leader_h

#include <janus/torder.h>

namespace janus {

class VT_with_leader :
        public torder_vs
{
  public:
    VT_with_leader();
    ~VT_with_leader();

    xmt::uuid_type vs_pub_recover();
    void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    void vs_pub_view_update();
    void vs_event_origin( const vtime&, const stem::Event& );
    void vs_event_derivative( const vtime&, const stem::Event& );
    void vs_pub_flush();

  private:
    void message( const stem::Event& );

    DECLARE_RESPONSE_TABLE( VT_with_leader, janus::torder_vs );
};

} // namespace janus

#endif // __leader_h

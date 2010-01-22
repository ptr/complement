// -*- C++ -*- Time-stamp: <10/01/22 10:31:25 ptr>

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
#include <fstream>

namespace janus {

class VT_with_leader :
        public torder_vs
{
  public:
    VT_with_leader( const char*, bool = false );
    ~VT_with_leader();

    xmt::uuid_type vs_pub_recover();
    void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    void vs_pub_view_update();
    void vs_pub_rec( const stem::Event& );
    void vs_pub_flush();

  private:
    void message( const stem::Event& );
    void sync_message( const stem::Event& );

    std::ofstream f;

    DECLARE_RESPONSE_TABLE( VT_with_leader, janus::torder_vs );
};

} // namespace janus

#endif // __leader_h

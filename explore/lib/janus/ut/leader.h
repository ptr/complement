// -*- C++ -*- Time-stamp: <10/02/05 15:57:28 ptr>

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
#include <string>

namespace janus {

class VT_with_leader :
        public torder_vs
{
  public:
    VT_with_leader( const char* );
    ~VT_with_leader();

    xmt::uuid_type vs_pub_recover();
    void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    void vs_pub_view_update();
    void vs_pub_rec( const stem::Event& );
    void vs_pub_flush();
    virtual void vs_pub_tord_rec( const stem::Event& );

  private:
    void message( const stem::Event& );
    void sync_message( const stem::Event& );

    std::ofstream f;
    std::string name;

    DECLARE_RESPONSE_TABLE( VT_with_leader, janus::torder_vs );
};

} // namespace janus

#endif // __leader_h

// -*- C++ -*- Time-stamp: <10/01/15 21:02:50 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __janus_torder_h
#define __janus_torder_h

#include <janus/vtime.h>

namespace janus {

class torder_vs :
    public basic_vs
{
  public:
    torder_vs();
    torder_vs( const char* info );
    ~torder_vs();

    int vs( const stem::Event& );

    template <class D>
    int vs( const stem::Event_base<D>& e )
      { return torder_vs::vs( stem::detail::convert<stem::Event_base<D>,stem::Event>()(e) ); }
    // void vs_send_flush();

  private:
    void vs_process( const stem::Event_base<vs_event>& );
    void vs_process_lk( const stem::Event_base<vs_event>& );

    DECLARE_RESPONSE_TABLE( torder_vs, basic_vs );
};

} // namespace janus

#endif // __janus_torder_h

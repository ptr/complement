// -*- C++ -*- Time-stamp: <08/06/30 19:07:02 yeti>

/*
 *
 * Copyright (c) 2007, 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __Convert_h
#define __Convert_h

#include <mt/mutex>
#include <mt/condition_variable>

#include <stem/Event.h>
#include <stem/EventHandler.h>

#include <stdint.h>
#include <string>

struct mess :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const;
    void net_pack( std::ostream& s ) const;
    void unpack( std::istream& s );
    void net_unpack( std::istream& s );

    mess()
      { }
    mess( const mess& m ) :
        super_id( m.super_id ),
        message( m.message )
      { }

    int32_t super_id;
    std::string message;
};

class Convert :
    public stem::EventHandler
{
  public:
    Convert();
    Convert( stem::addr_type id );
    Convert( stem::addr_type id, const char *info );
    ~Convert();

    void handler0();
    void handler1( const stem::Event& );
    void handler2( const stem::Event_base<mess>& );
    void handler3( const mess& );

    bool wait();

    static int v;

    std::string m2;
    std::string m3;

  private:
    static bool v_nz_check()
      { return v != 0; }

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;

    DECLARE_RESPONSE_TABLE( Convert, stem::EventHandler );
};

#define CONV_EV0 0x909
#define CONV_EV1 0x90a
#define CONV_EV2 0x90b
#define CONV_EV3 0x90c

#endif // __Convert_h

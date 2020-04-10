// -*- C++ -*-

/*
 *
 * Copyright (c) 2007-2009, 2020
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __Convert_h
#define __Convert_h

#include <mutex>
#include <condition_variable>

#include <stem/Event.h>
#include <stem/EventHandler.h>

#include <stdint.h>
#include <string>
#include <utility>

struct mess :
    public stem::__pack_base
{
    void pack( std::ostream& s ) const;
    void unpack( std::istream& s );

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
    typedef std::pair<int32_t,std::string> compaund_type;

    Convert();
    Convert( stem::addr_type id );
    Convert( stem::addr_type id, const char *info );
    ~Convert();

    void handler0();
    void handler1( const stem::Event& );
    void handler2( const stem::Event_base<mess>& );
    void handler3( const mess& );
    void handler4( const stem::EventVoid& );
    void handler5( const std::string& );
    void handler6( const stem::Event_base<compaund_type>& );

    bool wait();

    static int v;

    std::string m2;
    std::string m3;

  private:
    static bool v_nz_check()
      { return v != 0; }

    std::mutex mtx;
    std::condition_variable cnd;

    DECLARE_RESPONSE_TABLE( Convert, stem::EventHandler );
};

#define CONV_EV0 0x909
#define CONV_EV1 0x90a
#define CONV_EV2 0x90b
#define CONV_EV3 0x90c
#define CONV_EV4 0x90d
#define CONV_EV5 0x90e
#define CONV_EV6 0x90f

#endif // __Convert_h

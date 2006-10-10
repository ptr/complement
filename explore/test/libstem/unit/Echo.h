// -*- C++ -*- Time-stamp: <06/10/10 15:42:36 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __Echo_h
#define __Echo_h

#include <string>
#include <mt/xmt.h>
#include <stem/EventHandler.h>
// #include <stem/Names.h>
// #include <list>

class StEMecho :
    public stem::EventHandler
{
  public:
    StEMecho();
    StEMecho( stem::addr_type id );
    StEMecho( stem::addr_type id, const char * );

    void echo( const stem::Event& );
    void regme( const stem::Event& );

  private:
    DECLARE_RESPONSE_TABLE( StEMecho, stem::EventHandler );
};

class EchoClient :
    public stem::EventHandler
{
  public:
    EchoClient();
    EchoClient( stem::addr_type id );
    EchoClient( stem::addr_type id, const char *info );
    ~EchoClient();

    void handler1( const stem::Event& );

    void wait();

    const std::string mess;

  private:
    xmt::Condition cnd;

    DECLARE_RESPONSE_TABLE( EchoClient, stem::EventHandler );
};

#define NODE_EV_ECHO 0x903

#endif // __Echo_h

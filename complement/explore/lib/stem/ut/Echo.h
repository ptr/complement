// -*- C++ -*- Time-stamp: <08/06/30 13:15:07 yeti>

/*
 * Copyright (c) 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __Echo_h
#define __Echo_h

#include <string>
#include <mt/condition_variable>
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

    std::tr2::condition_event cnd;

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

    bool wait();

    const std::string mess;

  private:
    std::tr2::condition_event cnd;

    DECLARE_RESPONSE_TABLE( EchoClient, stem::EventHandler );
};

class PeerClient :
    public stem::EventHandler
{
  public:
    PeerClient();
    PeerClient( stem::addr_type id );
    PeerClient( const char *info );
    PeerClient( stem::addr_type id, const char *info );
    ~PeerClient();

    void handler1( const stem::Event& );

    bool wait();

    const std::string mess;

  private:
    std::tr2::condition_event cnd;

    DECLARE_RESPONSE_TABLE( PeerClient, stem::EventHandler );
};

#define NODE_EV_ECHO  0x903
#define NODE_EV_REGME 0x904

#endif // __Echo_h

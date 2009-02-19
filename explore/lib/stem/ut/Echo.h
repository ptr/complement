// -*- C++ -*- Time-stamp: <09/02/18 20:14:03 ptr>

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

class UglyEchoSrv :
    public stem::EventHandler
{
  public:
    UglyEchoSrv() { }
    
    UglyEchoSrv( stem::addr_type id ) :
        EventHandler( id )
      { }
      
    UglyEchoSrv( stem::addr_type id, const char* info ) :
        EventHandler( id, info )
      { }

    void echo( const stem::Event& );

  private:
    DECLARE_RESPONSE_TABLE( UglyEchoSrv, stem::EventHandler );
};

class UglyEchoClient :
    public stem::EventHandler
{
  public:
    UglyEchoClient() :
        rsp_count(0)
      { }
    
    UglyEchoClient( stem::addr_type id ) : 
        EventHandler( id ),
        rsp_count(0)
      { }
      
    UglyEchoClient( stem::addr_type id, const char *info ) :
        EventHandler( id, info ),
        rsp_count(0)
      { }

    void handler1( const stem::Event& );
    bool wait();

    std::tr2::mutex lock;
    std::string mess;
    std::list< std::string > rsp; 
    int rsp_count;

  private:
    std::tr2::condition_variable cnd;
    std::tr2::mutex mtx;
    
    struct rsp_count_8 
    {
      rsp_count_8( UglyEchoClient& m ) :
          me( m )
        { }

      bool operator()() const
        { return me.rsp_count == 8; }

      UglyEchoClient& me;
    };
    
    /*
    struct rsp_not_empty
    {
        rsp_not_empty( UglyEchoClient& m ) :
            me( m )
          { }

        bool operator()() const
          { return !me.rsp.empty(); }

        UglyEchoClient& me;
    };
    */
    
    DECLARE_RESPONSE_TABLE( UglyEchoClient, stem::EventHandler );
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

class EchoLast :
    public stem::EventHandler
{
  public:
    EchoLast();
    EchoLast( stem::addr_type id );
    EchoLast( stem::addr_type id, const char * );

    void echo( const stem::Event& );
    void last( const stem::Event& );

    bool wait();

    std::tr2::condition_event cnd;

  private:
    DECLARE_RESPONSE_TABLE( EchoLast, stem::EventHandler );
};

class LastEvent :
    public stem::EventHandler
{
  public:
    LastEvent( stem::addr_type id, const char *info );
    ~LastEvent();

    void handler( const stem::Event& );
    void conformation( const stem::Event& );

    bool wait();

    const std::string mess;

  private:
    std::tr2::condition_event cnd;
    stem::addr_type peer;
    std::tr2::condition_event cnd_conf;

    DECLARE_RESPONSE_TABLE( LastEvent, stem::EventHandler );
};

#define NODE_EV_ECHO               0x903
#define NODE_EV_REGME              0x904
#define NODE_EV_LAST               0x905
#define NODE_EV_LAST_CONFORMATION  0x906

#endif // __Echo_h

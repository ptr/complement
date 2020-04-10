// -*- C++ -*-

/*
 *
 * Copyright (c) 2002, 2003, 2006-2008, 2020
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __NodeDL_h
#define __NodeDL_h

#include <mutex>
#include <condition_variable>
#include <stem/EventHandler.h>

class NodeDL :
    public stem::EventHandler
{
  public:
    NodeDL();
    NodeDL( stem::addr_type id );
    ~NodeDL();

    void handler1( const stem::Event& );

    bool wait();

    int v;

  private:
    std::mutex m;
    std::condition_variable cnd;
    
    struct check_v 
    {
      check_v( NodeDL& m ) :
          me( m )
        { }

      bool operator()() const
        { return me.v == 1; }

      NodeDL& me;
    };

    DECLARE_RESPONSE_TABLE( NodeDL, stem::EventHandler );
};

// the same as Node, just another class
class NewNodeDL :
    public stem::EventHandler
{
  public:
    NewNodeDL();
    NewNodeDL( stem::addr_type id );
    ~NewNodeDL();

    void handler1( const stem::Event& );

    bool wait();

    int v;

  private:
    std::mutex m;
    std::condition_variable cnd;
    
    struct check_v 
    {
      check_v( NewNodeDL& m ) :
          me( m )
        { }

      bool operator()() const
        { return me.v == 1; }

      NewNodeDL& me;
    };

    DECLARE_RESPONSE_TABLE( NewNodeDL, stem::EventHandler );
};

#define NODE_EV2 0x901

extern "C" void *create_NewNodeDL( stem::addr_type );
extern "C" int wait_NewNodeDL( void * );
extern "C" int v_NewNodeDL( void * );
extern "C" void destroy_NewNodeDL( void * );

#endif // __NodeDL_h

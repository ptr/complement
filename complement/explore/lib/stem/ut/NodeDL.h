// -*- C++ -*- Time-stamp: <07/07/11 21:47:58 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __NodeDL_h
#define __NodeDL_h

#include <mt/xmt.h>
#include <stem/EventHandler.h>

class NodeDL :
    public stem::EventHandler
{
  public:
    NodeDL();
    NodeDL( stem::addr_type id );
    ~NodeDL();

    void handler1( const stem::Event& );

    void wait();

    int v;

  private:
    xmt::condition cnd;

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

    void wait();

    int v;

  private:
    xmt::condition cnd;

    DECLARE_RESPONSE_TABLE( NewNodeDL, stem::EventHandler );
};

#define NODE_EV2 0x901

extern "C" void *create_NewNodeDL( unsigned );
extern "C" void wait_NewNodeDL( void * );
extern "C" int v_NewNodeDL( void * );
extern "C" void destroy_NewNodeDL( void * );

#endif // __NodeDL_h
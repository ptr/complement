// -*- C++ -*- Time-stamp: <06/09/29 22:53:43 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __Node_h
#define __Node_h

#include <mt/xmt.h>
#include <stem/EventHandler.h>

class Node :
    public stem::EventHandler
{
  public:
    Node();
    Node( stem::addr_type id );
    Node( stem::addr_type id, const char *info );
    ~Node();

    void handler1( const stem::Event& );

    void wait();

    int v;

  private:
    xmt::Condition cnd;

    DECLARE_RESPONSE_TABLE( Node, stem::EventHandler );
};

// the same as Node, just another class
class NewNode :
    public stem::EventHandler
{
  public:
    NewNode();
    NewNode( stem::addr_type id );
    ~NewNode();

    void handler1( const stem::Event& );

    void wait();

    int v;

  private:
    xmt::Condition cnd;

    DECLARE_RESPONSE_TABLE( NewNode, stem::EventHandler );
};

#define NODE_EV1 0x900

#endif // __Node_h

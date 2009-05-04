// -*- C++ -*- Time-stamp: <09/05/05 09:34:44 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __Node_h
#define __Node_h

#include <mt/mutex>
#include <mt/condition_variable>
#include <stem/EventHandler.h>

class Node :
    public stem::EventHandler
{
  public:
    Node();
    Node( stem::addr_type id );
    Node( stem::addr_type id, int );
    Node( stem::addr_type id, const char *info );
    ~Node();

    void handler1( const stem::Event& );

    bool wait();

    int v;

  private:
    std::tr2::mutex m;
    std::tr2::condition_variable cnd;

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

    bool wait();

    int v;

  private:
    std::tr2::mutex m;
    std::tr2::condition_variable cnd;

    DECLARE_RESPONSE_TABLE( NewNode, stem::EventHandler );
};

#define NODE_EV1 0x900

#endif // __Node_h

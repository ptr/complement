// -*- C++ -*- Time-stamp: <06/10/03 11:16:29 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __NameService_h
#define __NameService_h

#include <mt/xmt.h>
#include <stem/EventHandler.h>
#include <stem/Names.h>
#include <list>

class Naming :
    public stem::EventHandler
{
  public:
    Naming();
    Naming( stem::addr_type id );
    ~Naming();

    void names_list( const stem::NameRecord& );
    void names_name( const stem::NameRecord& );

    void wait();
    void reset()
      { cnd.set( false ); }

    std::list<stem::NameRecord> lst;

  private:
    xmt::Condition cnd;

    DECLARE_RESPONSE_TABLE( Naming, stem::EventHandler );
};

#endif // __NameService_h

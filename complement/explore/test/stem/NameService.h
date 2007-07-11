// -*- C++ -*- Time-stamp: <07/07/11 21:47:37 ptr>

/*
 * Copyright (c) 2006
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

    typedef stem::NameRecords<stem::gaddr_type,std::string> nsrecords_type;

    void names_list( const nsrecords_type& );
    void names_name( const nsrecords_type& );

    void wait();
    void reset()
      { cnd.set( false ); }

    nsrecords_type::container_type lst;

  private:
    xmt::condition cnd;

    DECLARE_RESPONSE_TABLE( Naming, stem::EventHandler );
};

#endif // __NameService_h

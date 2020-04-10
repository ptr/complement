// -*- C++ -*-

/*
 * Copyright (c) 2006-2009, 2020
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __NameService_h
#define __NameService_h

#include <mt/condition_variable>
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

    typedef stem::NameRecords<stem::addr_type,std::string> nsrecords_type;

    void names_list( const nsrecords_type& );
    void names_name( const nsrecords_type& );

    bool wait();
    void reset()
      { cnd.reset(); }

    nsrecords_type::container_type lst;

  private:
    std::tr2::condition_event cnd;

    DECLARE_RESPONSE_TABLE( Naming, stem::EventHandler );
};

#endif // __NameService_h

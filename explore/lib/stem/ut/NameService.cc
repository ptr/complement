// -*- C++ -*-

/*
 * Copyright (c) 2006-2008, 2017
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <iostream>
#include <functional>
#include <iterator>

#include <stem/EDSEv.h>
#include "NameService.h"

#include <mt/date_time>

using namespace std;
using namespace stem;
using namespace std::tr2;

Naming::Naming() :
    EventHandler()
{
}

Naming::Naming( stem::addr_type id ) :
    EventHandler( id )
{
}

Naming::~Naming()
{
}

void Naming::names_list( const nsrecords_type& nr )
{
  copy( nr.container.begin(), nr.container.end(), back_insert_iterator<nsrecords_type::container_type>(lst) );

  cnd.notify_one();
}

void Naming::names_name( const nsrecords_type& nr )
{
  copy( nr.container.begin(), nr.container.end(), back_insert_iterator<nsrecords_type::container_type>(lst) );

  cnd.notify_one();
}

bool Naming::wait()
{
  return cnd.timed_wait( std::tr2::milliseconds( 500 ) );
}

DEFINE_RESPONSE_TABLE( Naming )
  EV_T_(0,EV_STEM_NS_LIST,names_list,nsrecords_type)
  EV_T_(0,EV_STEM_NS_NAME,names_name,nsrecords_type)
END_RESPONSE_TABLE

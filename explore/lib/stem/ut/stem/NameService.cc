// -*- C++ -*- Time-stamp: <06/11/29 10:50:21 ptr>

/*
 * Copyright (c) 2006, 2007
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

using namespace std;
using namespace stem;
using namespace xmt;

Naming::Naming() :
    EventHandler()
{
  cnd.set( false );
}

Naming::Naming( stem::addr_type id ) :
    EventHandler( id )
{
  cnd.set( false );
}

Naming::~Naming()
{
  // cnd.wait();
}

void Naming::names_list( const nsrecords_type& nr )
{
  copy( nr.container.begin(), nr.container.end(), back_insert_iterator<nsrecords_type::container_type>(lst) );

  cnd.set(true);
}

void Naming::names_name( const nsrecords_type& nr )
{
  copy( nr.container.begin(), nr.container.end(), back_insert_iterator<nsrecords_type::container_type>(lst) );

  cnd.set(true);
}

void Naming::wait()
{
  cnd.try_wait();
}

DEFINE_RESPONSE_TABLE( Naming )
  EV_T_(0,EV_STEM_NS_LIST,names_list,nsrecords_type)
  EV_T_(0,EV_STEM_NS_NAME,names_name,nsrecords_type)
END_RESPONSE_TABLE

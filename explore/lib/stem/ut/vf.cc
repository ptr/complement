// -*- C++ -*- Time-stamp: <09/06/05 00:45:08 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vf.h"

#include <mt/date_time>

// #include <iostream>

using namespace std;
using namespace stem;
using namespace std::tr2;

VF::VF( stem::addr_type id ) :
    EventHandler( id )
{
}

VF::~VF()
{
  this_thread::sleep( milliseconds(500) );
}

bool VF::Dispatch( const stem::Event& )
{
  // cerr << '.';
  this_thread::sleep( milliseconds(100) );

  s += '.';

  return true;
}

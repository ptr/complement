// -*- C++ -*- Time-stamp: <09/06/25 21:10:31 ptr>

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
#include <exam/suite.h>

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

VF1::VF1( stem::addr_type id ) :
    EventHandler( id )
{
  s = new char[42];
  EXAM_CHECK_ASYNC( s != 0 ); 
}

VF1::~VF1()
{
  if ( s != 0 ) {
    delete[] s;
    s = 0;
  }
}

bool VF1::Dispatch( const stem::Event& )
{
  this_thread::sleep( milliseconds(1500) );

  // ok, i want to use s here
  EXAM_CHECK_ASYNC( s != 0 );

  return true;
}

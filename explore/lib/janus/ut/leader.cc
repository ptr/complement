// -*- C++ -*- Time-stamp: <10/01/15 21:03:33 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"
#include "leader.h"

#include <iostream>
#include <janus/vtime.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

#include <algorithm>
#include <set>
#include <list>

#include <fstream>
#include <mt/uid.h>
#include <unistd.h>

namespace janus {

using namespace std;

VT_with_leader::VT_with_leader() :
    torder_vs()
{
  enable();
}

VT_with_leader::~VT_with_leader()
{
  disable();
}

xmt::uuid_type VT_with_leader::vs_pub_recover()
{
  return xmt::nil_uuid;
}

void VT_with_leader::vs_resend_from( const xmt::uuid_type&, const stem::addr_type& )
{
}

void VT_with_leader::vs_pub_view_update()
{
}

void VT_with_leader::vs_event_origin( const vtime&, const stem::Event& )
{
}

void VT_with_leader::vs_event_derivative( const vtime&, const stem::Event& )
{
}

void VT_with_leader::vs_pub_flush()
{
}

void VT_with_leader::message( const stem::Event& ev )
{
  // mess = ev.value();

//  EXAM_CHECK_ASYNC( (ev.flags() & stem::__Event_Base::vs) != 0 );

//  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
//  pass = true;
//  cnd.notify_one();
}

#define EV_SAMPLE      0x9010

DEFINE_RESPONSE_TABLE( VT_with_leader )
  EV_EDS( ST_NULL, EV_SAMPLE, message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::leader)
{
  return EXAM_RESULT;
}

} // namespace janus

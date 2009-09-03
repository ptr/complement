// -*- C++ -*- Time-stamp: <09/09/03 09:30:13 ptr>

/*
 *
 * Copyright (c) 2008-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"

#include <iostream>
#include <janus/vtime.h>

namespace janus {

using namespace std;

vtime_operations::vtime_operations()
{
//  try {
//    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
//    b2 = new ( shm_b.allocate( 1 ) ) xmt::__barrier<true>();
//  }
//  catch ( const xmt::shm_bad_alloc& err ) {
//    b2 = 0;
    // err.what();
//  }
}

vtime_operations::~vtime_operations()
{
//  if ( b2 ) {
//    b2->~__barrier<true>();
//    shm_b.deallocate( b2, 1 );
//  }
//  seg.deallocate();
}

int EXAM_IMPL(vtime_operations::vt_compare)
{
  const janus::addr_type p0(xmt::uid());
  const janus::addr_type p1(xmt::uid());
  const janus::addr_type p2(xmt::uid());
  const janus::addr_type p3(xmt::uid());

  vtime vt1;
  vtime vt2;

  vt1[p1] = 1;
  vt1[p2] = 1;

  vt2[p1] = 1;
  vt2[p2] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( vt2 <= vt1 );
  EXAM_CHECK( vt1 >= vt2 );
  EXAM_CHECK( vt2 >= vt1 );

  vt2[p3] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( !(vt2 <= vt1) );
  EXAM_CHECK( vt2 >= vt1 );

  vt1.clear();
  vt2.clear();

  vt1[p1] = 1;

  vt2[p1] = 1;
  vt2[p3] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( !(vt2 <= vt1) );
  
  vt1[p2] = 1;

  EXAM_CHECK( !(vt1 <= vt2) );
  EXAM_CHECK( !(vt2 <= vt1) );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::vt_add)
{
  const janus::addr_type p1(xmt::uid());
  const janus::addr_type p2(xmt::uid());
  const janus::addr_type p3(xmt::uid());

  vtime vt1;
  vtime vt2;
  vtime vt3;
  vtime vt4;

  vt1[p1] = 1;
  vt1[p2] = 1;

  vt3 = vt1 + vt2;

  EXAM_CHECK( vt1 <= vt3 );
  EXAM_CHECK( vt3 <= vt1 );

  vt2[p2] = 1;

  vt3 = vt1 + vt2;

  vt4[p1] = 1;
  vt4[p2] = 2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt4.clear();

  vt2[p3] = 1;

  vt3 = vt1 + vt2;

  vt4[p1] = 1;
  vt4[p2] = 2;
  vt4[p3] = 1;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::vt_diff)
{
  const janus::addr_type p1(xmt::uid());
  const janus::addr_type p2(xmt::uid());
  const janus::addr_type p3(xmt::uid());

  vtime vt1;
  vtime vt2;
  vtime vt3;
  vtime vt4;

  vt1[p1] = 1;
  vt1[p2] = 1;

  vt3 = vt1 - vt2;

  EXAM_CHECK( vt1 <= vt3 );
  EXAM_CHECK( vt3 <= vt1 );

  vt2[p1] = 1;

  vt3 = vt1 - vt2;

  vt4[p2] = 1;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2[p2] = 1;

  vt4.clear();

  vt3 = vt1 - vt2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2.clear();

  vt2[p3] = 1;
  
  try {
    vt3 = vt1 - vt2;
    EXAM_ERROR( "Virtual Times are incomparable" );
  }
  catch ( const std::range_error& err ) {
    EXAM_CHECK( true );
  }

  vt2.clear();

  vt2[p2] = 2;

  try {
    vt3 = vt1 - vt2;
    EXAM_ERROR( "Virtual Times are incomparable" );
  }
  catch ( const std::range_error& err ) {
    EXAM_CHECK( true );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::vt_max)
{
  const janus::addr_type p1(xmt::uid());
  const janus::addr_type p2(xmt::uid());
  const janus::addr_type p3(xmt::uid());

  vtime vt1;
  vtime vt2;
  vtime vt3;
  vtime vt4;

  vt1[p1] = 1;
  vt1[p2] = 1;

  vt3 = vt1;
  janus::sup( vt3, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[p1] = 1;
  
  vt3 = vt1;
  janus::sup( vt3, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[p2] = 1;

  vt3 = vt1;
  janus::sup( vt3, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[p3] = 1;

  vt3 = vt1;
  janus::sup( vt3, vt2 );

  vt4[p1] = 1;
  vt4[p2] = 1;
  vt4[p3] = 1;
 
  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2.clear();

  vt2[p1] = 1;
  vt2[p2] = 2;

  vt4.clear();

  vt3 = vt1;
  janus::sup( vt3, vt2 );

  vt4[p1] = 1;
  vt4[p2] = 2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2[p3] = 4;

  vt3 = vt1;
  janus::sup( vt3, vt2 );

  vt4[p3] = 4;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::gvt_add)
{
  const janus::addr_type p0(xmt::uid());
  const janus::addr_type p1(xmt::uid());
  const janus::addr_type p2(xmt::uid());

  const gid_type g0 = xmt::uid();
  const gid_type g1 = xmt::uid();

  {
    gvtime gvt1;
    gvtime gvt2;

    vtime vt1;
    vtime vt2;

    vt1[p1] = 1;
    vt1[p2] = 1;

    vt2[p1] = 1;
    vt2[p2] = 1;

    gvt1[g0] = vt1;
    gvt2[g0] = vt2;

    gvt1 += gvt2;

    EXAM_CHECK( gvt1[g0][p1] == 2 );
    EXAM_CHECK( gvt1[g0][p2] == 2 );
    EXAM_CHECK( gvt1[g0][p0] == 0 );
    EXAM_CHECK( gvt1[g1][p1] == 0 );
    EXAM_CHECK( gvt1[g1][p2] == 0 );
  }
  {
    gvtime gvt1;
    gvtime gvt2;

    vtime vt1;
    vtime vt2;

    vt1[p1] = 1;
    vt1[p2] = 1;

    vt2[p1] = 1;
    vt2[p2] = 1;

    gvt1[g0] = vt1;
    gvt2[g1] = vt2;

    gvt1 += gvt2;

    EXAM_CHECK( gvt1[g0][p1] == 1 );
    EXAM_CHECK( gvt1[g0][p2] == 1 );
    EXAM_CHECK( gvt1[g0][p0] == 0 );
    EXAM_CHECK( gvt1[g1][p1] == 1 );
    EXAM_CHECK( gvt1[g1][p2] == 1 );
  }
  {
    gvtime gvt1;

    vtime vt1;
    vtime vt2;

    vt1[p1] = 1;
    vt1[p2] = 1;

    vt2[p1] = 1;
    vt2[p2] = 1;

    gvt1[g0] = vt1;

    gvt1 += make_pair( g1, vt2 );

    EXAM_CHECK( gvt1[g0][p1] == 1 );
    EXAM_CHECK( gvt1[g0][p2] == 1 );
    EXAM_CHECK( gvt1[g0][p0] == 0 );
    EXAM_CHECK( gvt1[g1][p1] == 1 );
    EXAM_CHECK( gvt1[g1][p2] == 1 );
  }

  return EXAM_RESULT;
}

} // namespace janus

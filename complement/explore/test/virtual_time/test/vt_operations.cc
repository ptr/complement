// -*- C++ -*- Time-stamp: <07/07/27 10:42:55 ptr>

#include "vt_operations.h"

// #include <boost/lexical_cast.hpp>

#include <iostream>
#include <vtime.h>

using namespace vt;
using namespace std;

int EXAM_IMPL(vtime_operations::vt_compare)
{
  const oid_type t0(0);
  const oid_type t1(1);
  const oid_type t2(2);
  const oid_type t3(3);

  vtime_type vt1;
  vtime_type vt2;

  vt1[t1] = 1;
  vt1[t2] = 1;

  vt2[t1] = 1;
  vt2[t2] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( vt2 <= vt1 );
  EXAM_CHECK( vt1 >= vt2 );
  EXAM_CHECK( vt2 >= vt1 );

  vt2[t3] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( !(vt2 <= vt1) );
  EXAM_CHECK( vt2 >= vt1 );

  vt1.clear();
  vt2.clear();

  vt1[t1] = 1;

  vt2[t1] = 1;
  vt2[t3] = 1;

  EXAM_CHECK( vt1 <= vt2 );
  EXAM_CHECK( !(vt2 <= vt1) );
  
  vt1[t2] = 1;

  EXAM_CHECK( !(vt1 <= vt2) );
  EXAM_CHECK( !(vt2 <= vt1) );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::vt_add)
{
  const oid_type t1(1);
  const oid_type t2(2);
  const oid_type t3(3);

  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1[t1] = 1;
  vt1[t2] = 1;

  vt3 = vt1 + vt2;

  EXAM_CHECK( vt1 <= vt3 );
  EXAM_CHECK( vt3 <= vt1 );

  vt2[t2] = 1;

  vt3 = vt1 + vt2;

  vt4[t1] = 1;
  vt4[t2] = 2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt4.clear();

  vt2[t3] = 1;

  vt3 = vt1 + vt2;

  vt4[t1] = 1;
  vt4[t2] = 2;
  vt4[t3] = 1;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::vt_diff)
{
  const oid_type t1(1);
  const oid_type t2(2);
  const oid_type t3(3);

  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1[t1] = 1;
  vt1[t2] = 1;

  vt3 = vt1 - vt2;

  EXAM_CHECK( vt1 <= vt3 );
  EXAM_CHECK( vt3 <= vt1 );

  vt2[t1] = 1;

  vt3 = vt1 - vt2;

  vt4[t2] = 1;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2[t2] = 1;

  vt4.clear();

  vt3 = vt1 - vt2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2.clear();

  vt2[t3] = 1;
  
  try {
    vt3 = vt1 - vt2;
    EXAM_ERROR( "Virtual Times are incomparable" );
  }
  catch ( const std::range_error& err ) {
    EXAM_CHECK( true );
  }

  vt2.clear();

  vt2[t2] = 2;

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
  const oid_type t1(1);
  const oid_type t2(2);
  const oid_type t3(3);

  vtime_type vt1;
  vtime_type vt2;
  vtime_type vt3;
  vtime_type vt4;

  vt1[t1] = 1;
  vt1[t2] = 1;

  vt3 = vt1;
  vt::sup( vt3, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[t1] = 1;
  
  vt3 = vt1;
  vt::sup( vt3, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[t2] = 1;

  vt3 = vt1;
  vt::sup( vt3, vt2 );

  EXAM_CHECK( vt3 <= vt1 );
  EXAM_CHECK( vt1 <= vt3 );

  vt2[t3] = 1;

  vt3 = vt1;
  vt::sup( vt3, vt2 );

  vt4[t1] = 1;
  vt4[t2] = 1;
  vt4[t3] = 1;
 
  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2.clear();

  vt2[t1] = 1;
  vt2[t2] = 2;

  vt4.clear();

  vt3 = vt1;
  vt::sup( vt3, vt2 );

  vt4[t1] = 1;
  vt4[t2] = 2;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  vt2[t3] = 4;

  vt3 = vt1;
  vt::sup( vt3, vt2 );

  vt4[t3] = 4;

  EXAM_CHECK( vt3 <= vt4 );
  EXAM_CHECK( vt4 <= vt3 );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::gvt_add)
{
  const oid_type t0(0);
  const oid_type t1(1);
  const oid_type t2(2);

  {
    gvtime_type gvt1;
    gvtime_type gvt2;

    vtime_type vt1;
    vtime_type vt2;

    vt1[t1] = 1;
    vt1[t2] = 1;

    vt2[t1] = 1;
    vt2[t2] = 1;

    gvt1[0] = vt1;
    gvt2[0] = vt2;

    gvt1 += gvt2;

    EXAM_CHECK( gvt1[0][t1] == 2 );
    EXAM_CHECK( gvt1[0][t2] == 2 );
    EXAM_CHECK( gvt1[0][t0] == 0 );
    EXAM_CHECK( gvt1[1][t1] == 0 );
    EXAM_CHECK( gvt1[1][t2] == 0 );
  }
  {
    gvtime_type gvt1;
    gvtime_type gvt2;

    vtime_type vt1;
    vtime_type vt2;

    vt1[t1] = 1;
    vt1[t2] = 1;

    vt2[t1] = 1;
    vt2[t2] = 1;

    gvt1[0] = vt1;
    gvt2[1] = vt2;

    gvt1 += gvt2;

    EXAM_CHECK( gvt1[0][t1] == 1 );
    EXAM_CHECK( gvt1[0][t2] == 1 );
    EXAM_CHECK( gvt1[0][t0] == 0 );
    EXAM_CHECK( gvt1[1][t1] == 1 );
    EXAM_CHECK( gvt1[1][t2] == 1 );
  }
  {
    gvtime_type gvt1;

    vtime_type vt1;
    vtime_type vt2;

    vt1[t1] = 1;
    vt1[t2] = 1;

    vt2[t1] = 1;
    vt2[t2] = 1;

    gvt1[0] = vt1;

    gvt1 += make_pair( 1, vt2 );

    EXAM_CHECK( gvt1[0][t1] == 1 );
    EXAM_CHECK( gvt1[0][t2] == 1 );
    EXAM_CHECK( gvt1[0][t0] == 0 );
    EXAM_CHECK( gvt1[1][t1] == 1 );
    EXAM_CHECK( gvt1[1][t2] == 1 );
  }

  return EXAM_RESULT;
}

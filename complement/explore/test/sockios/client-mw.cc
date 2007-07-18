// -*- C++ -*- Time-stamp: <05/12/01 20:29:05 ptr>

/*
 *
 * Copyright (c) 2002, 2003
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.1
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#include <exam/suite.h>

#include <string>
#include <sockios/sockstream>
#include <iostream>
#include <iomanip>
#include <mt/xmt.h>

#include "client-mw.h"
#include "message.h"

using namespace std;
using namespace xmt;

int EXAM_IMPL(ClientMassWrite::client_proc)
{
  using namespace test_area;

  EXAM_MESSAGE( "Client start" );
  for ( int i_close = 0; i_close < ni2; ++i_close ) {
    std::sockstream sock( "localhost", ::port );

    for ( int i_send = 0; i_send < ni1; ++i_send ) {
      sock.write( bin_buff1, bin_buff1_size );
    }

    EXAM_CHECK( sock.good() );
  }
  EXAM_MESSAGE( "Client end" );

  return EXAM_RESULT;
}

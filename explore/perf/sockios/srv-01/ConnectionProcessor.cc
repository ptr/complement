// -*- C++ -*- Time-stamp: <02/09/09 11:15:55 ptr>

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include "ConnectionProcessor.h"
#include <iostream>

using namespace std;

// int nc = 0;

ConnectionProcessor::ConnectionProcessor( std::sockstream& s )
{
}

void ConnectionProcessor::connect( std::sockstream& s )
{

  return;
}

void ConnectionProcessor::close()
{
  // cerr << ++nc << endl;
}

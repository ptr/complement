// -*- C++ -*- Time-stamp: <02/12/01 11:33:59 ptr>

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include "ConnectionProcessor.h"
// #include "Message.h"
#include <mt/xmt.h>

using namespace std;
using namespace __impl;

int count = 0;
int scount = 0;

extern Condition e;
extern int bs;

ConnectionProcessor::ConnectionProcessor( std::sockstream& s )
{
  OUT_MSG( "Server see connection" );
//  string msg;
//  s >> msg;

//  ::count += msg.size();
}

void ConnectionProcessor::connect( std::sockstream& s )
{
  // string msg;
  // s >> msg;
  char *b = new char [bs];
  s.read( b, bs );

  // ::count += msg.size();
  ::count += bs;
  ++::scount;

#if 0
  string omsg( "- " );

  getline( s, msg );
  if ( msg != message ) {
    OUT_MSG( "Server: Strings don't match!" );
    exit( 1 ); 
  }
  omsg += msg;
  omsg += " -";
  s << omsg << endl;
#endif

  return;
}

void ConnectionProcessor::close()
{
  OUT_MSG( "Server see: client close connection (" << ::count
           << "/" << ::scount << ")" );
  e.signal();
}

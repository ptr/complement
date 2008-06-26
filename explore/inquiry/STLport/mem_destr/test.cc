// -*- C++ -*- Time-stamp: <02/09/25 12:11:17 ptr>

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
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <iostream>

using namespace std;

class One
{
  public:
    One()
      { cerr << "One" << endl; }
    ~One()
      { cerr << "~One" << endl; }
};

class Two
{
  public:
    One one;
};

// Two two;
char buf[sizeof(Two)];

int main( int argc, char * const *argv )
{
  new (&buf) Two();
  cerr << "Hello, world!" << endl;
  ((Two *)&buf)->~Two();

  return 0;
}

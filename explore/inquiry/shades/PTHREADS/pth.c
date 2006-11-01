// -*- C++ -*- Time-stamp: <03/07/04 22:29:04 ptr>

/*
 * Copyright (c) 2003
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.2
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

#include <pthread.h>
#include <unistd.h>

int main( int argc, char **argv )
{
#ifdef _PTHREADS
  printf( "_PTHREADS defined\n" );
#else
  printf( "_PTHREADS not defined\n" );
#endif

  return 0;
}

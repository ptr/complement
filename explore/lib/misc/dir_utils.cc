// -*- C++ -*- Time-stamp: <00/09/08 18:01:32 ptr>

/*
 *
 * Copyright (c) 1997
 * Petr Ovchenkov
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
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#include <config/feature.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
#include "aux/dir_utils.h"
#include "aux/directory.h"
#else
#include "aux_/dir_utils.h"
#include "aux_/directory.h"
#endif

using namespace __STD;

extern "C" char *tempnam( const char *, const char * ); // not part of STDC

string mkunique_dir( const string& base, const string& prefix )
{
  char *tmp = ::tempnam( base.c_str(), prefix.c_str() );
  mkdir( tmp, S_IRWXU | S_IRWXG | S_IRWXO );
  string unique( tmp );
  free( tmp );

  return unique;
}

void rm_dir_all( const string& d )
{
  struct stat buf;
  static const string dot( "." );
  static const string dotdot( ".." );

  for ( dir_it current(d); current != dir_it(); ++current ) {
    if (*current != dot && *current != dotdot ) {
      stat( (*current).c_str(), &buf );
      if ( buf.st_mode & S_IFDIR ) {
	rm_dir_all( (*current).c_str() );
      } else {
	unlink( (*current).c_str() );
      }
    }
  }
  rmdir( d.c_str() );
}

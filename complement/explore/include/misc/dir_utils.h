// -*- C++ -*- Time-stamp: <01/03/19 16:32:47 ptr>

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

#ifndef __dir_utils_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>

#ifndef  _UNISTD_H
#include <unistd.h>
#endif

std::string mkunique_dir( const std::string& base, const std::string& prefix );

inline void rm_dir( const std::string& d ) { ::rmdir( d.c_str() ); }
void rm_dir_all( const std::string& d );

#endif // __dir_utils_h

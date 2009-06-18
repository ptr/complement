// -*- C++ -*- Time-stamp: <09/06/18 11:59:58 ptr>

/*
 * Copyright (c) 2006, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <mt/shm.h>
#include <mt/system_error>
#include <iostream>

namespace xmt {

using namespace std;

static char _buf[128];

static const std::string rstring( "shared memory" );

const char* shm_bad_alloc::what() const throw()
{
  if ( err == -1 ) {
    return "shared memory: address already assigned";
  }
  if ( err == -2 ) {
    return "shared memory: segment not allocated";
  }
  if ( err == -3 ) {
    return "shared memory: not enough space left in shared memory segment";
  }
  strncpy( _buf, std::system_error( err, std::get_posix_category(), rstring ).what(), sizeof(_buf) );
  _buf[sizeof(_buf)-1] = 0; // null term, even if room wasn't enough
  return _buf;
}

} // namespace xmt

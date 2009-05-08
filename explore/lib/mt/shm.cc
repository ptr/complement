// -*- C++ -*- Time-stamp: <09/05/08 11:09:47 ptr>

/*
 * Copyright (c) 2006, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <mt/shm.h>
#include <mt/system_error>

namespace xmt {

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
  return std::system_error( err, std::get_posix_category(), rstring ).what();
}

} // namespace xmt

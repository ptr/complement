// -*- C++ -*- Time-stamp: <06/12/20 11:44:26 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <mt/shm.h>

namespace xmt {

const char *shm_bad_alloc::what() const throw()
{
  switch ( err ) {
    case 0:
      return "All fine";
    case EACCES:
      return "Permission denied";
    case EBADF:
      return "Bad file number";
    case EFAULT:
      return "Bad address";
    case ELOOP:
      return "Too many symbolic links encountered";
    case ENAMETOOLONG:
      return "File name too long";
    case ENOENT:
      return "No such file or directory or segment";
    case ENOMEM:
      return "Out of memory";
    case ENOTDIR:
      return "Not a directory";
    case EEXIST:
      return "File exists";
    case EINVAL:
      return "Invalid argument";
    case ENFILE:
      return "File table overflow";
    case ENOSPC:
      return "No space left on device";
    case EPERM:
      return "Operation not permitted";
    case EIDRM:
      return "Identifier removed";
    case EOVERFLOW:
      return "Value too large for defined data type";

    case -1:
      return "Address already assigned";
    case -2:
      return "Shared memory segment not allocated";
    case -3:
      return "Not enough space left in shared memory segment";
    case -4:
      return "Reference file exists";
  }

  return "unknown";
}

} // namespace xmt

// -*- C++ -*-

/*
 * Copyright (c) 2004, 2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <config/feature.h>
#include <mt/lfstream>

#if defined(__unix) || defined(__unix__)
# include <fcntl.h>
# include <sys/file.h>
# include <sys/types.h>
# include <unistd.h>
# include <errno.h>
#endif /* __unix || __unix__ */

#if !defined(STLPORT) && defined(__GNUC__) && (__GNUC__ >= 5)
#include <system_error>
#else
#include <mt/system_error>
#endif
#include <mt/mutex>

namespace std {

namespace detail {

#  if !defined(__FreeBSD__) && !defined(__OpenBSD__)
static struct flock _flw = { F_WRLCK, SEEK_SET, 0, 0, 0 };
static struct flock _flr = { F_RDLCK, SEEK_SET, 0, 0, 0 };
static struct flock _flu = { F_UNLCK, SEEK_SET, 0, 0, 0 };
#  else
static struct flock _flw = { 0, 0, 0, F_WRLCK, SEEK_SET };
static struct flock _flr = { 0, 0, 0, F_RDLCK, SEEK_SET };
static struct flock _flu = { 0, 0, 0, F_UNLCK, SEEK_SET };
#  endif

void __flock::rdlock( int fd )
{
  int res = ::fcntl( fd, F_SETLKW, &_flr );
  while ( res < 0 ) {
    if ( errno == EINTR ) {
      res = ::fcntl( fd, F_SETLKW, &_flr );
    } else {
      throw std::tr2::lock_error( res );
    }
  }
}

void __flock::lock( int fd )
{
  int res = ::fcntl( fd, F_SETLKW, &_flw );
  while ( res < 0 ) {
    if ( errno == EINTR ) {
      res = ::fcntl( fd, F_SETLKW, &_flw );
    } else {
      throw std::tr2::lock_error( res );
    }
  }
}

bool __flock::try_rdlock( int fd )
{
  int res = ::fcntl( fd, F_SETLK, &_flr );
  if ( res != 0 ) {
    if ( res == EBUSY ) {
      return false;
    }
    throw std::tr2::lock_error( res );
  }
  return true;
}

bool __flock::try_lock( int fd )
{
  int res = ::fcntl( fd, F_SETLK, &_flw );
  if ( res != 0 ) {
    if ( res == EBUSY ) {
      return false;
    }
    throw std::tr2::lock_error( res );
  }
  return true;
}

void __flock::unlock( int fd )
{
  ::fcntl( fd, F_SETLKW, &_flu );
}

} // namespace detail

} // namespace std

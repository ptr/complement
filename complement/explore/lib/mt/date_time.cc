// -*- C++ -*- Time-stamp: <08/02/24 18:58:05 ptr>

/*
 * Copyright (c) 2002, 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 * Derived from original <mt/time.h> of 'complement' project
 * [http://complement.sourceforge.net]
 * to make it close to JTC1/SC22/WG21 working draft
 * [http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2497.html]
 */

#include <mt/date_time>
#include <ctime>

namespace std {

namespace tr2 {

const nanoseconds::tick_type nanoseconds::ticks_per_second = 1000000000LL;
const nanoseconds::tick_type nanoseconds::seconds_per_tick = 0LL;
const bool nanoseconds::is_subsecond = true;

system_time nanoseconds::operator +( const system_time& r ) const
{
  return (r + *this);
}

const microseconds::tick_type microseconds::ticks_per_second = 1000000LL;
const microseconds::tick_type microseconds::seconds_per_tick = 0LL;
const bool microseconds::is_subsecond = true;

system_time microseconds::operator +( const system_time& r ) const
{
  return (r + *this);
}

const milliseconds::tick_type milliseconds::ticks_per_second = 1000LL;
const milliseconds::tick_type milliseconds::seconds_per_tick = 0LL;
const bool milliseconds::is_subsecond = true;

system_time milliseconds::operator +( const system_time& r ) const
{
  return (r + *this);
}

const seconds::tick_type seconds::ticks_per_second = 1LL;
const seconds::tick_type seconds::seconds_per_tick = 1LL;
const bool seconds::is_subsecond = false;

system_time seconds::operator +( const system_time& r ) const
{
  return (r + *this);
}

const minutes::tick_type minutes::ticks_per_second = 0LL;
const minutes::tick_type minutes::seconds_per_tick = 60LL;
const bool minutes::is_subsecond = false;

system_time minutes::operator +( const system_time& r ) const
{
  return (r + *this);
}

const hours::tick_type hours::ticks_per_second = 0LL;
const hours::tick_type hours::seconds_per_tick = 3600LL;
const bool hours::is_subsecond = false;

system_time hours::operator +( const system_time& r ) const
{
  return (r + *this);
}

const system_time::tick_type system_time::ticks_per_second = 1000000000ULL;
const system_time::tick_type system_time::seconds_per_tick = 0ULL;
const bool system_time::is_subsecond = true;

system_time get_system_time()
{
#if defined(__linux) || defined(__FreeBSD__) || defined(__OpenBSD__)
  timeval tv;
  gettimeofday( &tv, 0 );
  return system_time( tv.tv_sec * nanoseconds::ticks_per_second + tv.tv_usec * 1000LL, system_time::_adopt_t() );
#elif defined( WIN32 )
  union {
    FILETIME ft; // 100 ns intervals since Jan 1 1601 (UTC)
      __int64 t;
  } ft;
  GetSystemTimeAsFileTime( &ft.ft );
  return system_time( (ft.t - 11644473600LL) * 100LL, system_time::_adopt_t() ); // 60 * 60 * 24 * 134774, 1970 - 1601
#elif defined(__sun) || defined(__hpux)
  ::timespec tv;
  clock_gettime( CLOCK_REALTIME, &tv );
  return system_time( tv.tv_sec * nanoseconds::ticks_per_second + tv.tv_nsec, system_time::_adopt_t() ) ;
#else
#error "You should implement OS-dependent precise clock"
#endif
}

} // namespace tr2

} // namespace std

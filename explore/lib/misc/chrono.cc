// -*- C++ -*- Time-stamp: <2011-05-02 17:40:13 ptr>

/*
 * Copyright (c) 2002, 2006-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 * Derived from original <mt/time.h> of 'complement' project
 * [http://complement.sourceforge.net]
 * to make it close to JTC1/SC22/WG21 C++ 0x working draft
 * [http://www.open-std.org/Jtc1/sc22/wg21/docs/papers/2011/n3291.pdf]
 */

#include <misc/chrono>

#if !((defined(STLPORT) && (_STLPORT_VERSION < 0x520)) || \
      (defined(__GNUC__) && defined(__FIT_CPP_0X)))

namespace std {

namespace chrono {

const bool system_clock::is_steady = false;

system_clock::time_point system_clock::now() /* nothrow */ throw()
{
  //timeval tv;
  //::gettimeofday( &tv, 0 );
  //return system_time( tv.tv_sec * nanoseconds::ticks_per_second + tv.tv_usec * 1000LL, system_time::_adopt_t() );

#ifdef __unix
  ::timespec tv;
  clock_gettime( CLOCK_REALTIME, &tv );
 
  return time_point( duration( tv.tv_nsec + tv.tv_sec * period::den ) /* seconds(tv.tv_sec) + nanoseconds(tv.tv_nsec) */ );
#elif defined( WIN32 )
  union {
    FILETIME ft; // 100 ns intervals since Jan 1 1601 (UTC)
      __int64 t;
  } ft;
  GetSystemTimeAsFileTime( &ft.ft );

  return time_point( duration( ft.t - 11644473600LL) ); // 60 * 60 * 24 * 134774, 1970 - 1601
#else
# error "You should implement OS-dependent precise clock"
#endif
}

time_t system_clock::to_time_t( const system_clock::time_point& t ) /* noexcept */ throw()
{
#ifdef __unix
  return t.time_since_epoch().count() / period::den;
#elif defined( WIN32 )
#else
# error "You should implement OS-dependent precise clock"
#endif
}

system_clock::time_point system_clock::from_time_t( time_t t ) /* noexcept */ throw()
{
#ifdef __unix
  return time_point( duration_cast<duration>( seconds(t) ) );
#elif defined( WIN32 )
#else
# error "You should implement OS-dependent precise clock"
#endif
}

const bool steady_clock::is_steady = true;

steady_clock::time_point steady_clock::now() /* nothrow */ throw()
{
#ifdef __unix
  ::timespec tv;
  clock_gettime( CLOCK_MONOTONIC, &tv );
 
  return time_point( duration( tv.tv_nsec + tv.tv_sec * period::den ) /* seconds(tv.tv_sec) + nanoseconds(tv.tv_nsec) */ );
#elif defined( WIN32 )
#else
# error "You should implement OS-dependent precise clock"
#endif
}

} // namespace chrono

} // namespace std

#endif // !STLPORT && !__FIT_CPP_0X

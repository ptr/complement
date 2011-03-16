// -*- C++ -*- Time-stamp: <2011-03-16 12:03:55 ptr>

/*
 * Copyright (c) 2006, 2008-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <algorithm>
#include <iterator>

#include <mt/uid.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <stdexcept>
#include <mt/system_error>
#include <misc/md5.h>

#include <sys/sysctl.h>
#include <linux/sysctl.h>

#include <iostream>

namespace std {

std::ostream& operator <<( std::ostream& s, const xmt::uuid_type& uid )
{
#ifdef STLPORT
  std::ios_base::fmtflags f = s.flags( 0 );
#else // i.e. libstdc++
  std::ios_base::fmtflags f = s.flags( static_cast<std::ios_base::fmtflags>(0) );
#endif
  s << hex << setfill('0')
    << setw(2) << static_cast<unsigned>(uid.u.b[0])
    << setw(2) << static_cast<unsigned>(uid.u.b[1])
    << setw(2) << static_cast<unsigned>(uid.u.b[2])
    << setw(2) << static_cast<unsigned>(uid.u.b[3]) << '-'
    << setw(2) << static_cast<unsigned>(uid.u.b[4])
    << setw(2) << static_cast<unsigned>(uid.u.b[5]) << '-'
    << setw(2) << static_cast<unsigned>(uid.u.b[6])
    << setw(2) << static_cast<unsigned>(uid.u.b[7]) << '-'
    << setw(2) << static_cast<unsigned>(uid.u.b[8])
    << setw(2) << static_cast<unsigned>(uid.u.b[9]) << '-'
    << setw(2) << static_cast<unsigned>(uid.u.b[10])
    << setw(2) << static_cast<unsigned>(uid.u.b[11])
    << setw(2) << static_cast<unsigned>(uid.u.b[12])
    << setw(2) << static_cast<unsigned>(uid.u.b[13])
    << setw(2) << static_cast<unsigned>(uid.u.b[14])
    << setw(2) << static_cast<unsigned>(uid.u.b[15]);
  s.flags( f );

  return s;
}

std::istream& operator >>( std::istream& s, xmt::uuid_type& uid )
{
#ifdef STLPORT
  std::ios_base::fmtflags f = s.flags( 0 );
#else // i.e. libstdc++
  std::ios_base::fmtflags f = s.flags( static_cast<std::ios_base::fmtflags>(0) );
#endif

  if ( (f & std::ios_base::skipws) != 0 ) {
    s.setf( std::ios_base::skipws );
  }

  std::istream::sentry __sentry( s ); // skip whitespace

  if ( !__sentry ) {
    s.flags( f );
    return s;
  }

  s >> hex >> uid.u.i[0];
#ifdef _LITTLE_ENDIAN
  swap( uid.u.b[0], uid.u.b[3] );
  swap( uid.u.b[1], uid.u.b[2] );
#endif

  char c = ' ';

  s >> c;

  if ( c != '-' ) {
    s.putback( c );
    s.setstate( std::ios_base::failbit );
    s.flags( f );
    return s;
  }
  c = ' ';

  s >> uid.u.s[2];
#ifdef _LITTLE_ENDIAN
  swap( uid.u.b[4], uid.u.b[5] );
#endif

  s >> c;

  if ( c != '-' ) {
    s.putback( c );
    s.setstate( std::ios_base::failbit );
    s.flags( f );
    return s;
  }
  c = ' ';

  s >> uid.u.s[3];
#ifdef _LITTLE_ENDIAN
  swap( uid.u.b[6], uid.u.b[7] );
#endif

  s >> c;

  if ( c != '-' ) {
    s.putback( c );
    s.setstate( std::ios_base::failbit );
    s.flags( f );
    return s;
  }
  c = ' ';

  s >> uid.u.s[4];
#ifdef _LITTLE_ENDIAN
  swap( uid.u.b[8], uid.u.b[9] );
#endif

  s >> c;

  if ( c != '-' ) {
    s.putback( c );
    s.setstate( std::ios_base::failbit );
    s.flags( f );
    return s;
  }

  stringstream ss;
  
  std::copy_n( istreambuf_iterator<char>(s), 4, ostreambuf_iterator<char>(ss) );
  ss.put( ' ' );
  std::copy_n( istreambuf_iterator<char>(s), 8, ostreambuf_iterator<char>(ss) );

  ss >> hex >> uid.u.s[5];
#ifdef _LITTLE_ENDIAN
  swap( uid.u.b[10], uid.u.b[11] );
#endif
  ss >> uid.u.i[3];
#ifdef _LITTLE_ENDIAN
  swap( uid.u.b[12], uid.u.b[15] );
  swap( uid.u.b[13], uid.u.b[14] );
#endif

  s.flags( f );

  return s;
}

} // namespace std

namespace xmt {

const uuid_type nil_uuid = { { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } } }; // NIL UUID

namespace detail {

using namespace std;
using namespace xmt;

struct __uid_init
{
    __uid_init();

    static uuid_type _host_id;
    int err;
};

uuid_type __uid_init::_host_id;

__uid_init::__uid_init() :
     err( 0 )
{
  static size_t n = sizeof(uuid_type);
  static int mib[] = { CTL_KERN, KERN_RANDOM, RANDOM_BOOT_ID };

  err = sysctl( mib, sizeof(mib)/sizeof(mib[0]), &_host_id, &n, (void*)0, 0 );
}

} // namespace detail

using namespace std;

uuid_type::operator string() const
{
  ostringstream s;

  s << *this;

  return s.str();
}

const char *hostid_str() throw (runtime_error)
{
  static std::string _uid;

  if ( _uid.size() == 0 ) {
    stringstream s;
    s << hostid();
    _uid = s.str();
  }

  return _uid.data();
}

const xmt::uuid_type& hostid() throw (runtime_error)
{
  static detail::__uid_init _uid;

  if ( _uid.err != 0 ) {
    throw system_error( _uid.err, get_posix_category(), "boot_id" );
  }

  return detail::__uid_init::_host_id;
}

std::string uid_str() throw (runtime_error)
{
  stringstream s;

  s << uid();

  return s.str();
}

xmt::uuid_type uid() throw (runtime_error)
{
  uuid_type id;
  static size_t n = sizeof(uuid_type);

  static int mib[] = { CTL_KERN, KERN_RANDOM, RANDOM_UUID };

  int err = sysctl( mib, sizeof(mib)/sizeof(mib[0]), &id, &n, (void*)0, 0 );

  if ( err ) {
    throw system_error( err, get_posix_category(), "uuid" );
  }

  return id;
}

xmt::uuid_type uid_md5( const void* s, size_t n )
{
  uuid_type id;

  MD5_CTX context;

  MD5Init( &context );
  MD5Update( &context, reinterpret_cast<const uint8_t*>(s), n );
  MD5Final( id.u.b, &context );

  return id;
}

} // namespace xmt

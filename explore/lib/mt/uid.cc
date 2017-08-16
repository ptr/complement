// -*- C++ -*-

/*
 * Copyright (c) 2006, 2008-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <config/feature.h>

#include <algorithm>
#include <iterator>

#if !defined(STLPORT) && defined(__GNUC__) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
// for copy_n
# include <ext/algorithm>
#endif

#include <mt/uid.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <stdexcept>
#if !defined(STLPORT) && defined(__GNUC__) && (__GNUC__ >= 5)
#include <system_error>
#else
#include <mt/system_error>
#endif
#include <misc/md5.h>

#include <sys/sysctl.h>
#include <linux/sysctl.h>
#include <uuid/uuid.h>

#include <iostream>

namespace std {

#if !defined(STLPORT) && defined(__GNUC__) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
using __gnu_cxx::copy_n;
#endif

std::ostream& operator <<( std::ostream& s, const xmt::uuid_type& uid )
{
  char b[37];

  uuid_unparse_lower( uid.u.b, b );

  return s << b;

/*
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
*/
}

std::istream& operator >>( std::istream& s, xmt::uuid_type& uid )
{
#ifdef STLPORT
  std::ios_base::fmtflags f = s.flags( 0 );
#else // i.e. libstdc++
  std::ios_base::fmtflags f = s.flags( static_cast<std::ios_base::fmtflags>(0) );
#endif

  std::istream::sentry __sentry( s, (f & std::ios_base::skipws) == 0 ); // skip whitespace

  if ( !__sentry ) {
    s.flags( f );
    return s;
  }

  char b[37];

  copy_n( istreambuf_iterator<char>(s), 36, b );
  b[36] = '\0';
  if ( !s.fail() ) {
    if ( uuid_parse( b, uid.u.b ) < 0 ) {
      for (int r = 35; r >= 0; --r ) {
        s.putback( b[r] );
      }
      s.setstate( std::ios_base::failbit );
    }
  }

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
  ifstream boot_id( "/proc/sys/kernel/random/boot_id" );

  if ( boot_id.is_open() && boot_id.good() ) {
    boot_id >> _host_id;
  } else {
    static size_t n = sizeof(uuid_type);
    static int mib[] = { CTL_KERN, KERN_RANDOM, RANDOM_BOOT_ID };

    err = sysctl( mib, sizeof(mib)/sizeof(mib[0]), &_host_id, &n, (void*)0, 0 );
  }
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
  static char b[37] = "";

  if ( b[0] == 0 ) {
    uuid_unparse_lower( hostid().u.b, b );
  }

  return b;
}

const xmt::uuid_type& hostid() throw (runtime_error)
{
  static detail::__uid_init _uid;

  if ( _uid.err != 0 ) {
    throw system_error( _uid.err, system_category(), "boot_id" );
  }

  return detail::__uid_init::_host_id;
}

std::string uid_str() throw (runtime_error)
{
  char b[37];
  uuid_type id;

  uuid_generate_random( id.u.b );
  uuid_unparse_lower( id.u.b, b );

  return std::string( b, 36 );
}

xmt::uuid_type uid() throw (runtime_error)
{
  uuid_type id;

  uuid_generate_random( id.u.b );

  return id;
}

const uuid_type ns_url  = { { { 0x6b, 0xa7, 0xb8, 0x11, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 } } }; // UUID namespace URL
const uuid_type ns_oid  = { { { 0x6b, 0xa7, 0xb8, 0x12, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 } } }; // UUID namespace ISO OID
const uuid_type ns_x500 = { { { 0x6b, 0xa7, 0xb8, 0x14, 0x9d, 0xad, 0x11, 0xd1, 0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 } } }; // UUID namespace X.500 DN

xmt::uuid_type uid_md5( const void* s, size_t n )
{
  uuid_type id;

  MD5_CTX context;

  MD5Init( &context );
  MD5Update( &context, ns_oid.u.b, 16 ); // namespace ISO OID
  MD5Update( &context, reinterpret_cast<const uint8_t*>(s), n );
  MD5Final( id.u.b, &context );

  id.u.b[6] &= 0x0f;
  id.u.b[6] |= 0x30; // UUID version 3
  id.u.b[8] &= 0x3f;
  id.u.b[8] |= 0x80; // UUID variant DCE

  return id;
}

int uid_version( const xmt::uuid_type& id )
{
  return static_cast<int>(id.u.b[6] >> 4);
}

int uid_variant( const xmt::uuid_type& id )
{
  return static_cast<int>(id.u.b[8] >> 6);
}

} // namespace xmt

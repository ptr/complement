// -*- C++ -*- Time-stamp: <09/04/07 16:47:41 ptr>

/*
 * Copyright (c) 2006, 2008, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <mt/uid.h>
#include <mt/mutex>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <mt/system_error>
#include <misc/md5.h>

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
    s.setf( std::ios_base::failbit );
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
    s.setf( std::ios_base::failbit );
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
    s.setf( std::ios_base::failbit );
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
    s.setf( std::ios_base::failbit );
    s.flags( f );
    return s;
  }

  char buf[13];

  s.read( buf, 4 );
  buf[4] = ' ';
  s.read( buf + 5, 8 );

  stringstream ss( buf, 13 );

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

namespace detail {

using namespace std;
using namespace xmt;
using namespace std::tr2;

struct __uid_init
{
    __uid_init();

    static uuid_type _host_id;
    char _host_id_str[48]; // 37 really
    int err;
};

uuid_type __uid_init::_host_id;

static const char boot_id[] = "/proc/sys/kernel/random/boot_id";
static const char uu_id[] = "/proc/sys/kernel/random/uuid";

__uid_init::__uid_init() :
     err( 0 )
{
  int fd = ::open( boot_id, O_RDONLY );

  if ( (fd < 0) || (::read( fd, _host_id_str, 36 ) != 36 )) {
    err = errno;
    if ( fd >= 0 ) {
      ::close( fd );
    }
  } else {
    _host_id_str[36] = '\0';
    ::close( fd );

    stringstream s;
    s << _host_id_str[0]  << _host_id_str[1]  << ' '  
      << _host_id_str[2]  << _host_id_str[3]  << ' '
      << _host_id_str[4]  << _host_id_str[5]  << ' '
      << _host_id_str[6]  << _host_id_str[7]  << ' ' // -
      << _host_id_str[9]  << _host_id_str[10] << ' '
      << _host_id_str[11] << _host_id_str[12] << ' ' // -
      << _host_id_str[14] << _host_id_str[15] << ' '
      << _host_id_str[16] << _host_id_str[17] << ' ' // -
      << _host_id_str[19] << _host_id_str[20] << ' '
      << _host_id_str[21] << _host_id_str[22] << ' ' // -
      << _host_id_str[24] << _host_id_str[25] << ' '
      << _host_id_str[26] << _host_id_str[27] << ' '
      << _host_id_str[28] << _host_id_str[29] << ' '
      << _host_id_str[30] << _host_id_str[31] << ' '
      << _host_id_str[32] << _host_id_str[33] << ' '
      << _host_id_str[34] << _host_id_str[35];

    s >> hex;

    unsigned v[16];

    s >> v[0] >> v[1] >> v[2]  >> v[3]  >> v[4]  >> v[5]  >> v[6]  >> v[7]
      >> v[8] >> v[9] >> v[10] >> v[11] >> v[12] >> v[13] >> v[14] >> v[15];

    _host_id.u.b[0] = v[0];
    _host_id.u.b[1] = v[1];
    _host_id.u.b[2] = v[2];
    _host_id.u.b[3] = v[3];
    _host_id.u.b[4] = v[4];
    _host_id.u.b[5] = v[5];
    _host_id.u.b[6] = v[6];
    _host_id.u.b[7] = v[7];
    _host_id.u.b[8] = v[8];
    _host_id.u.b[9] = v[9];
    _host_id.u.b[10] = v[10];
    _host_id.u.b[11] = v[11];
    _host_id.u.b[12] = v[12];
    _host_id.u.b[13] = v[13];
    _host_id.u.b[14] = v[14];
    _host_id.u.b[15] = v[15];
  }
}

} // namespace detail

using namespace std;
using namespace std::tr2;

uuid_type::operator string() const
{
  ostringstream s;

  s << *this;

  return s.str();
}

const char *hostid_str() throw (runtime_error)
{
  static detail::__uid_init _uid;
  if ( _uid.err != 0 ) {
    throw system_error( _uid.err, get_posix_category(), detail::boot_id );
  }
  return _uid._host_id_str;
}

const xmt::uuid_type& hostid() throw (runtime_error)
{
  hostid_str();
  return detail::__uid_init::_host_id;
}

std::string uid_str() throw (runtime_error)
{
  char buf[37];

  int fd = ::open( detail::uu_id, O_RDONLY );
  if ( (fd < 0) || (::read( fd, buf, 37 ) != 37) ) {
    system_error se( errno, get_posix_category(), string( "Can't generate UID; " ) + detail::uu_id );
    if ( fd >= 0 ) {
      ::close( fd );
    }
    throw se;
    // return std::string();
  }
  ::close( fd );

  return std::string( buf, 36 );
}

xmt::uuid_type uid() throw (runtime_error)
{
  string tmp = uid_str();
  uuid_type id;

  stringstream s;
  s << tmp[0]  << tmp[1]  << ' '  
    << tmp[2]  << tmp[3]  << ' '
    << tmp[4]  << tmp[5]  << ' '
    << tmp[6]  << tmp[7]  << ' ' // -
    << tmp[9]  << tmp[10] << ' '
    << tmp[11] << tmp[12] << ' ' // -
    << tmp[14] << tmp[15] << ' '
    << tmp[16] << tmp[17] << ' ' // -
    << tmp[19] << tmp[20] << ' '
    << tmp[21] << tmp[22] << ' ' // -
    << tmp[24] << tmp[25] << ' '
    << tmp[26] << tmp[27] << ' '
    << tmp[28] << tmp[29] << ' '
    << tmp[30] << tmp[31] << ' '
    << tmp[32] << tmp[33] << ' '
    << tmp[34] << tmp[35];
    
  s >> hex;

  unsigned v[16];

  s >> v[0] >> v[1] >> v[2]  >> v[3]  >> v[4]  >> v[5]  >> v[6]  >> v[7]
    >> v[8] >> v[9] >> v[10] >> v[11] >> v[12] >> v[13] >> v[14] >> v[15];

  id.u.b[0] = v[0];
  id.u.b[1] = v[1];
  id.u.b[2] = v[2];
  id.u.b[3] = v[3];
  id.u.b[4] = v[4];
  id.u.b[5] = v[5];
  id.u.b[6] = v[6];
  id.u.b[7] = v[7];
  id.u.b[8] = v[8];
  id.u.b[9] = v[9];
  id.u.b[10] = v[10];
  id.u.b[11] = v[11];
  id.u.b[12] = v[12];
  id.u.b[13] = v[13];
  id.u.b[14] = v[14];
  id.u.b[15] = v[15];

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

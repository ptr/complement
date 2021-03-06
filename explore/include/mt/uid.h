// -*- C++ -*- Time-stamp: <2011-05-24 14:45:06 ptr>

/*
 * Copyright (c) 2006, 2008-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

/*
 * Refs:
 *  - RFC 4122 "A Universally Unique IDentifier (UUID) URN Namespace"
 *  - ISO/IEC 9834-8:2004
 */

#ifndef __mt_uid_h
#define __mt_uid_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <stdint.h>
#include <stdexcept>
#include <ostream>
#include <cstring>

#if (defined(STLPORT) && ((_STLPORT_MAJOR > 5) || ((_STLPORT_MAJOR == 5) && (_STLPORT_MINOR > 1)))) || (defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))) && defined(__GXX_EXPERIMENTAL_CXX0X__))
# include <type_traits>
#else
# include <misc/type_traits.h>
#endif

namespace xmt {

struct uuid_type
{
    union {
      uint8_t  b[16];
      uint16_t s[8];
      uint32_t i[4];
      uint64_t l[2];
    } u;

  
//    uuid_type()
//      { u.l[0] = 0; u.l[1] = 0; }

//    uuid_type( const uuid_type& uid )
//      {
        // u.i[0] = uid.u.i[0]; u.i[1] = uid.u.i[1];
        // u.i[2] = uid.u.i[2]; u.i[3] = uid.u.i[3];
//        u.l[0] = uid.u.l[0]; u.l[1] = uid.u.l[1];
//      }

    // uuid_type& operator =( const uuid_type& uid )
    //   { u.l[0] = uid.u.l[0]; u.l[1] = uid.u.l[1]; /* uuid_copy( u.b, uid.u.b ); */ return *this; }

#ifdef __FIT_CPP_0X
    uuid_type() = default;
    uuid_type( const uuid_type& uid ) = default;
    ~uuid_type() = default;

    uuid_type& operator =( const uuid_type& uid ) = default;
#endif // __FIT_CPP_0X


    bool operator ==( const uuid_type& uid ) const
      {
        // return u.i[0] == uid.u.i[0] && u.i[1] == uid.u.i[1] && u.i[2] == uid.u.i[2] && u.i[3] == uid.u.i[3];
        return u.l[0] == uid.u.l[0] && u.l[1] == uid.u.l[1];
        // return uuid_compare( u.b, uid.u.b ) == 0;
      }
    
    bool operator !=( const uuid_type& uid ) const
      {
        // return u.i[0] != uid.u.i[0] || u.i[1] != uid.u.i[1] || u.i[2] != uid.u.i[2] || u.i[3] != uid.u.i[3];
        return u.l[0] != uid.u.l[0] || u.l[1] != uid.u.l[1];
        // return uuid_compare( u.b, uid.u.b ) != 0;
      }

    bool operator <( const uuid_type& uid ) const
      {
        // return std::lexicographical_compare( u.i, u.i + 4, uid.u.i, uid.u.i + 4 );
        return u.l[0] < uid.u.l[0] ? true : u.l[0] > uid.u.l[0] ? false : (u.l[1] < uid.u.l[1]);
        // return uuid_compare( u.b, uid.u.b ) < 0;
      }

    bool operator >( const uuid_type& uid ) const
      {
        return u.l[0] > uid.u.l[0] ? true : u.l[0] < uid.u.l[0] ? false : (u.l[1] > uid.u.l[1]);
        // return uuid_compare( u.b, uid.u.b ) > 0;
      }

    operator std::string() const;

    void swap( uuid_type& r )
      {
        std::swap( u.l[0], r.u.l[0] );
        std::swap( u.l[1], r.u.l[1] );
      }
};

inline bool operator >=( const uuid_type& l, const uuid_type& r )
{ return !(l < r); }

inline bool operator <=( const uuid_type& l, const uuid_type& r )
{ return !(l > r); }

extern const uuid_type nil_uuid; // NIL UUID

// Hostid:

const char* hostid_str() throw (std::runtime_error);
const xmt::uuid_type& hostid() throw (std::runtime_error);

// UUID genration:

std::string uid_str() throw (std::runtime_error);
xmt::uuid_type uid() throw (std::runtime_error);

// Create UUID from MD5 signature:

xmt::uuid_type uid_md5( const void*, size_t );
inline xmt::uuid_type uid_md5( const char* s )
{ return xmt::uid_md5( s, strlen(s) ); }
inline xmt::uuid_type uid_md5( const std::string& s )
{ return xmt::uid_md5( s.data(), s.size() ); }

// UUID version

int uid_version( const xmt::uuid_type& );
int uid_variant( const xmt::uuid_type& );

} // namespace xmt

namespace std {

std::ostream& operator <<( std::ostream&, const xmt::uuid_type& );
std::istream& operator >>( std::istream&, xmt::uuid_type& );

# if (defined(__GNUC__) && !defined(__FIT_CPP_0X))
namespace tr1 {
# endif

template <>
struct is_trivial<xmt::uuid_type> :
    public integral_constant<bool, true>
{ };

template <>
struct is_standard_layout<xmt::uuid_type> :
    public integral_constant<bool, true>
{ };

template <>
struct is_pod<xmt::uuid_type> :
    public integral_constant<bool, true>
{ };

# if (defined(__GNUC__) && !defined(__FIT_CPP_0X))
} // namespace tr1
# endif

template <>
inline void swap( xmt::uuid_type& l, xmt::uuid_type& r )
{ l.swap(r); }

} // namespace std

#endif // __mt_uid_h

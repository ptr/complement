// -*- C++ -*- Time-stamp: <00/05/10 16:28:19 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __EvPack_h
#define __EvPack_h

#ident "$SunId$ %Q%"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifdef WIN32
#include <winsock.h>
#  if defined(BIGENDIAN) && (BIGENDIAN > 0)
#    define _BIG_ENDIAN
#  else
#    define _LITTLE_ENDIAN
#  endif
#else // !WIN32
#  ifndef __Linux
#    include <sys/isa_defs.h>
#  else
#    include <sys/types.h>
#    if (__BYTE_ORDER==__LITTLE_ENDIAN)
#      define _LITTLE_ENDIAN
#    else
#      define _BIG_ENDIAN
#    endif
#  endif
#endif // WIN32

#ifdef _LITTLE_ENDIAN
#  include <algorithm>
#endif

#include <istream>
#include <ostream>

namespace EDS {

#ifndef _MSC_VER

template <int S>
class __byte_swapper
{
  public:
    static void __byte_swap( char *i )
      { __STD::swap( *i, *(i+S-1) );  __byte_swapper<S-2>::__byte_swap( i + 1 ); }
};

template <>
class __byte_swapper<1>
{
  public:
    static void __byte_swap( char * )
      { }
};

template <>
class __byte_swapper<0>
{
  public:
    static void __byte_swap( char * )
      { }
};

template <class T>
class __net_converter
{
#ifdef _LITTLE_ENDIAN
    union __swapper
    {
        char  byte[sizeof(T)];
        T value;
    };
#endif
  public:
    static T to_net( const T& x )
      {
#ifdef _LITTLE_ENDIAN
        __swapper _sw;
        _sw.value = x;
        __byte_swapper<sizeof(T)>::__byte_swap( _sw.byte );
/*
        int i = 0;
        int j = sizeof(T) - 1;
        while ( i < j ) {
          swap( _sw.byte[i++], _sw.byte[j--] );          
        }
*/
        return _sw.value;
#else
        return x;
#endif
      }

    static T from_net( const T& x )
      {
#ifdef _LITTLE_ENDIAN
        __swapper _sw;
        _sw.value = x;
        __byte_swapper<sizeof(T)>::__byte_swap( _sw.byte );
/*
        int i = 0;
        int j = sizeof(T) - 1;
        while ( i < j ) {
          swap( _sw.byte[i++], _sw.byte[j--] );          
        }
*/
        return _sw.value;
#else
        return x;
#endif
      }
};

template <>
class __net_converter<char>
{
  public:
    static char to_net( const char& x )
      { return x; }

    static char from_net( const char& x )
      { return x; }
};

template <>
class __net_converter<unsigned char>
{
  public:
    static unsigned char to_net( const unsigned char& x )
      { return x; }

    static unsigned char from_net( const unsigned char& x )
      { return x; }
};

template <>
class __net_converter<signed char>
{
  public:
    static signed char to_net( const signed char& x )
      { return x; }

    static signed char from_net( const signed char& x )
      { return x; }
};

template <class T>
T to_net( const T& x )
{ return EDS::__net_converter<T>::to_net( x ); }

template <class T>
T from_net( const T& x )
{ return EDS::__net_converter<T>::from_net( x ); }

#else // !_MSC_VER

inline
char to_net( const char& x )
{ return x; }

inline
signed char to_net( const signed char& x )
{ return x; }

inline
unsigned char to_net( const unsigned char& x )
{ return x; }

inline
short to_net( const short& x )
{ return htons( x ); }

inline
unsigned short to_net( const unsigned short& x )
{ return htons( x ); }

inline
int to_net( const int& x )
{ return htonl( x ); }

inline
unsigned to_net( const unsigned& x )
{ return htonl( x ); }

inline
long to_net( const long& x )
{ return htonl( x ); }

inline
unsigned long to_net( const unsigned long& x )
{ return htonl( x ); }

inline
int from_net( const int& x )
{ return ntohl( x ); }

inline
unsigned from_net( const unsigned& x )
{ return ntohl( x ); }

inline
long from_net( const long& x )
{ return ntohl( x ); }

inline
unsigned long from_net( const unsigned long& x )
{ return ntohl( x ); }

#endif

struct __pack_base
{
    // To be released for data structure you want pass via EDS:
    __PG_DECLSPEC virtual void pack( __STD::ostream& ) const = 0 ;
    __PG_DECLSPEC virtual void unpack( __STD::istream& ) = 0 ;
    __PG_DECLSPEC virtual void net_pack( __STD::ostream& ) const = 0 ;
    __PG_DECLSPEC virtual void net_unpack( __STD::istream& ) = 0 ;

    // basic types

    // string
    static __PG_DECLSPEC void __net_unpack( __STD::istream& s, __STD::string& str );
    static __PG_DECLSPEC void __net_pack( __STD::ostream& s, const __STD::string& str );
    static __PG_DECLSPEC void __unpack( __STD::istream& s, __STD::string& str );
    static __PG_DECLSPEC void __pack( __STD::ostream& s, const __STD::string& str );
    // int
    static void __net_unpack( __STD::istream& s, int& x )
      {
        s.read( (char *)&x, sizeof(int) );
        x = EDS::from_net( x );
      }
    static void __net_pack( __STD::ostream& s, int x )
      {
        x = EDS::to_net( x );
        s.write( (const char *)&x, 4 );
      }
    static void __unpack( __STD::istream& s, int& x )
      { s.read( (char *)&x, sizeof(int) ); }
    static void __pack( __STD::ostream& s, int x )
      { s.write( (const char *)&x, 4 ); }
    // unsigned
    static void __net_unpack( __STD::istream& s, unsigned& x )
      {
        s.read( (char *)&x, sizeof(unsigned) );
        x = EDS::from_net( x );
      }
    static void __net_pack( __STD::ostream& s, unsigned x )
      {
        x = EDS::to_net( x );
        s.write( (const char *)&x, sizeof(unsigned) );
      }
    static void __unpack( __STD::istream& s, unsigned& x )
      { s.read( (char *)&x, sizeof(unsigned) ); }
    static void __pack( __STD::ostream& s, unsigned x )
      { s.write( (const char *)&x, sizeof(unsigned) ); }
    // long
    static void __net_unpack( __STD::istream& s, long& x )
      {
        s.read( (char *)&x, sizeof(long) );
        x = EDS::from_net( x );
      }
    static void __net_pack( __STD::ostream& s, long x )
      {
        x = EDS::to_net( x );
        s.write( (const char *)&x, sizeof(long) );
      }
    static void __unpack( __STD::istream& s, long& x )
      { s.read( (char *)&x, sizeof(long) ); }
    static void __pack( __STD::ostream& s, long x )
      { s.write( (const char *)&x, sizeof(long) ); }
    // unsigned long
    static void __net_unpack( __STD::istream& s, unsigned long& x )
      {
        s.read( (char *)&x, sizeof(unsigned long) );
        x = EDS::from_net( x );
      }
    static void __net_pack( __STD::ostream& s, unsigned long x )
      {
        x = EDS::to_net( x );
        s.write( (const char *)&x, sizeof(unsigned long) );
      }
    static void __unpack( __STD::istream& s, unsigned long& x )
      { s.read( (char *)&x, sizeof(unsigned long) ); }
    static void __pack( __STD::ostream& s, unsigned long x )
      { s.write( (const char *)&x, sizeof(unsigned long) ); }
    // char
    static void __net_unpack( __STD::istream& s, char& x )
      { s.read( (char *)&x, sizeof(char) ); }
    static void __net_pack( __STD::ostream& s, char x )
      { s.write( (const char *)&x, 1 ); }
    static void __unpack( __STD::istream& s, char& x )
      { s.read( (char *)&x, sizeof(char) ); }
    static void __pack( __STD::ostream& s, char x )
      { s.write( (const char *)&x, 1 ); }
    // signed char
    static void __net_unpack( __STD::istream& s, signed char& x )
      { s.read( (char *)&x, sizeof(signed char) ); }
    static void __net_pack( __STD::ostream& s, signed char x )
      { s.write( (const char *)&x, 1 ); }
    static void __unpack( __STD::istream& s, signed char& x )
      { s.read( (char *)&x, sizeof(signed char) ); }
    static void __pack( __STD::ostream& s, signed char x )
      { s.write( (const char *)&x, 1 ); }
    // unsigned char
    static void __net_unpack( __STD::istream& s, unsigned char& x )
      { s.read( (char *)&x, sizeof(unsigned char) ); }
    static void __net_pack( __STD::ostream& s, unsigned char x )
      { s.write( (const char *)&x, 1 ); }
    static void __unpack( __STD::istream& s, unsigned char& x )
      { s.read( (char *)&x, sizeof(unsigned char) ); }
    static void __pack( __STD::ostream& s, unsigned char x )
      { s.write( (const char *)&x, 1 ); }
};

}

#endif // __EvPack_h

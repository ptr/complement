// -*- C++ -*- Time-stamp: <05/12/29 23:57:13 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003, 2005
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __stem_EvPack_h
#define __stem_EvPack_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#ifdef _LITTLE_ENDIAN
#  include <algorithm>
#endif

#include <iostream>

#ifdef WIN32
#include <winsock.h>
#endif

namespace stem {

#ifndef _MSC_VER

template <int S>
class __byte_swapper
{
  public:
    static void __byte_swap( char *i )
      { std::swap( *i, *(i+S-1) );  __byte_swapper<S-2>::__byte_swap( i + 1 ); }
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
{ return stem::__net_converter<T>::to_net( x ); }

template <class T>
T from_net( const T& x )
{ return stem::__net_converter<T>::from_net( x ); }

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
    // To be released for data structure you want pass via stem:
    __FIT_DECLSPEC virtual void pack( std::ostream& ) const = 0 ;
    __FIT_DECLSPEC virtual void unpack( std::istream& ) = 0 ;
    __FIT_DECLSPEC virtual void net_pack( std::ostream& ) const = 0 ;
    __FIT_DECLSPEC virtual void net_unpack( std::istream& ) = 0 ;

    // basic types

    // string
    static __FIT_DECLSPEC void __net_unpack( std::istream& s, std::string& str );
    static __FIT_DECLSPEC void __net_pack( std::ostream& s, const std::string& str );
    static __FIT_DECLSPEC void __unpack( std::istream& s, std::string& str );
    static __FIT_DECLSPEC void __pack( std::ostream& s, const std::string& str );
    // int
    static void __net_unpack( std::istream& s, int& x )
      {
        s.read( (char *)&x, sizeof(int) );
        x = stem::from_net( x );
      }
    static void __net_pack( std::ostream& s, int x )
      {
        x = stem::to_net( x );
        s.write( (const char *)&x, sizeof(int) );
      }
    static void __unpack( std::istream& s, int& x )
      { s.read( (char *)&x, sizeof(int) ); }
    static void __pack( std::ostream& s, int x )
      { s.write( (const char *)&x, sizeof(int) ); }
    // unsigned
    static void __net_unpack( std::istream& s, unsigned& x )
      {
        s.read( (char *)&x, sizeof(unsigned) );
        x = stem::from_net( x );
      }
    static void __net_pack( std::ostream& s, unsigned x )
      {
        x = stem::to_net( x );
        s.write( (const char *)&x, sizeof(unsigned) );
      }
    static void __unpack( std::istream& s, unsigned& x )
      { s.read( (char *)&x, sizeof(unsigned) ); }
    static void __pack( std::ostream& s, unsigned x )
      { s.write( (const char *)&x, sizeof(unsigned) ); }
    // long
    static void __net_unpack( std::istream& s, long& x )
      {
        s.read( (char *)&x, sizeof(long) );
        x = stem::from_net( x );
      }
    static void __net_pack( std::ostream& s, long x )
      {
        x = stem::to_net( x );
        s.write( (const char *)&x, sizeof(long) );
      }
    static void __unpack( std::istream& s, long& x )
      { s.read( (char *)&x, sizeof(long) ); }
    static void __pack( std::ostream& s, long x )
      { s.write( (const char *)&x, sizeof(long) ); }
    // unsigned long
    static void __net_unpack( std::istream& s, unsigned long& x )
      {
        s.read( (char *)&x, sizeof(unsigned long) );
        x = stem::from_net( x );
      }
    static void __net_pack( std::ostream& s, unsigned long x )
      {
        x = stem::to_net( x );
        s.write( (const char *)&x, sizeof(unsigned long) );
      }
    static void __unpack( std::istream& s, unsigned long& x )
      { s.read( (char *)&x, sizeof(unsigned long) ); }
    static void __pack( std::ostream& s, unsigned long x )
      { s.write( (const char *)&x, sizeof(unsigned long) ); }
    // char
    static void __net_unpack( std::istream& s, char& x )
      { s.read( (char *)&x, sizeof(char) ); }
    static void __net_pack( std::ostream& s, char x )
      { s.write( (const char *)&x, 1 ); }
    static void __unpack( std::istream& s, char& x )
      { s.read( (char *)&x, sizeof(char) ); }
    static void __pack( std::ostream& s, char x )
      { s.write( (const char *)&x, 1 ); }
    // signed char
    static void __net_unpack( std::istream& s, signed char& x )
      { s.read( (char *)&x, sizeof(signed char) ); }
    static void __net_pack( std::ostream& s, signed char x )
      { s.write( (const char *)&x, 1 ); }
    static void __unpack( std::istream& s, signed char& x )
      { s.read( (char *)&x, sizeof(signed char) ); }
    static void __pack( std::ostream& s, signed char x )
      { s.write( (const char *)&x, 1 ); }
    // unsigned char
    static void __net_unpack( std::istream& s, unsigned char& x )
      { s.read( (char *)&x, sizeof(unsigned char) ); }
    static void __net_pack( std::ostream& s, unsigned char x )
      { s.write( (const char *)&x, 1 ); }
    static void __unpack( std::istream& s, unsigned char& x )
      { s.read( (char *)&x, sizeof(unsigned char) ); }
    static void __pack( std::ostream& s, unsigned char x )
      { s.write( (const char *)&x, 1 ); }
    // short
    static void __net_unpack( std::istream& s, short& x )
      {
        s.read( (char *)&x, sizeof(short) );
        x = stem::from_net( x );
      }
    static void __net_pack( std::ostream& s, short x )
      {
        x = stem::to_net( x );
        s.write( (const char *)&x, sizeof(short) );
      }
    static void __unpack( std::istream& s, short& x )
      { s.read( (char *)&x, sizeof(short) ); }
    static void __pack( std::ostream& s, short x )
      { s.write( (const char *)&x, sizeof(short) ); }
    // unsigned short
    static void __net_unpack( std::istream& s, unsigned short& x )
      {
        s.read( (char *)&x, sizeof(unsigned short) );
        x = stem::from_net( x );
      }
    static void __net_pack( std::ostream& s, unsigned short x )
      {
        x = stem::to_net( x );
        s.write( (const char *)&x, sizeof(unsigned short) );
      }
    static void __unpack( std::istream& s, unsigned short& x )
      { s.read( (char *)&x, sizeof(unsigned short) ); }
    static void __pack( std::ostream& s, unsigned short x )
      { s.write( (const char *)&x, sizeof(unsigned short) ); }
};

} // stem

namespace EDS = stem;

#endif // __stem_EvPack_h

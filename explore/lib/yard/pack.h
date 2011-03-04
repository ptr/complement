/*
 *
 * Copyright (c) 1997-1999, 2002-2003, 2005-2006, 2009
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __yard_pack_h
#define __yard_pack_h

#include <mt/uid.h>
#include <sstream>

namespace yard {

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
{ return __net_converter<T>::to_net( x ); }

template <class T>
T from_net( const T& x )
{ return __net_converter<T>::from_net( x ); }

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

struct std_packer
{
    template <class T>
    static void unpack( std::istream& s, T& x )
    {
        s.read( (char *)&x, sizeof(T) );
        x = from_net( x );
    }

    template <class T>
    static void pack( std::ostream& s, T x )
    {
        x = to_net( x );
        s.write( (const char *)&x, sizeof(T) );
    }

    static __FIT_DECLSPEC void unpack( std::istream& s, xmt::uuid_type& u )
    {
        s.read( reinterpret_cast<char*>(u.u.b), sizeof(u.u.b) );
    }

    static __FIT_DECLSPEC void pack( std::ostream& s, const xmt::uuid_type& u )
    {
        s.write( reinterpret_cast<const char*>(u.u.b), sizeof(u.u.b) );
    }
};

template <typename T, typename Packer>
struct packer_traits
{
    static unsigned int min_size();
    static unsigned int max_size();
    static unsigned int size(const T& value)
    {
        std::stringstream ss;
        Packer::pack(ss, value);
        return ss.tellp();
    }
    static bool have_max_size();
};

template <typename T>
struct packer_traits<T, std_packer>
{
    static unsigned int min_size()
    {
        return sizeof(T);
    }

    static unsigned int max_size()
    {
        return sizeof(T);
    }

    static unsigned int size(const T& value)
    {
        return sizeof(T);
    }

    static bool have_max_size()
    {
        return true;
    }
};

void uuid_to_long_number(const xmt::uuid_type& u, uint8_t* long_number);
void long_number_to_uuid(const uint8_t* long_number, xmt::uuid_type& u);

struct uuid_packer_exp
{
    static __FIT_DECLSPEC void unpack( std::istream& s, xmt::uuid_type& u );
    static __FIT_DECLSPEC void pack( std::ostream& s, const xmt::uuid_type& u );
};

struct varint_packer
{
    static __FIT_DECLSPEC void unpack( std::istream& s, uint32_t& num );
    static __FIT_DECLSPEC void pack( std::ostream& s, const uint32_t& num );
};

}

#endif

// -*- C++ -*- Time-stamp: <99/05/24 15:04:53 ptr>
#ifndef __EvPack_h
#define __EvPack_h

#ident "$SunId$ %Q%"

#ifdef WIN32
#  include <winsock2.h>
#  if defined(BIGENDIAN) && (BIGENDIAN > 0)
#    define _BIG_ENDIAN
#  else
#    define _LITTLE_ENDIAN
#  endif
#  include <win_config.h>
#else
#  include <sys/isa_defs.h>
#endif

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
    __DLLEXPORT virtual void pack( std::ostream& ) const = 0 ;
    __DLLEXPORT virtual void unpack( std::istream& ) = 0 ;
    __DLLEXPORT virtual void net_pack( std::ostream& ) const = 0 ;
    __DLLEXPORT virtual void net_unpack( std::istream& ) = 0 ;

    static __DLLEXPORT void __net_unpack( std::istream& s, std::string& str );
    static __DLLEXPORT void __net_pack( std::ostream& s, const std::string& str );
    static __DLLEXPORT void __unpack( std::istream& s, std::string& str );
    static __DLLEXPORT void __pack( std::ostream& s, const std::string& str );
    static void __net_unpack( std::istream& s, int& x )
      {
        s.read( (char *)&x, sizeof(int) );
        x = EDS::from_net( x );
      }
    static void __net_pack( std::ostream& s, int x )
      {
        x = EDS::to_net( x );
        s.write( (const char *)&x, 4 );
      }
    static void __unpack( std::istream& s, int& x )
      { s.read( (char *)&x, sizeof(int) ); }
    static void __pack( std::ostream& s, int x )
      { s.write( (const char *)&x, 4 ); }
    static void __net_unpack( std::istream& s, unsigned& x )
      {
        s.read( (char *)&x, sizeof(unsigned) );
        x = EDS::from_net( x );
      }
    static void __net_pack( std::ostream& s, unsigned x )
      {
        x = EDS::to_net( x );
        s.write( (const char *)&x, sizeof(unsigned) );
      }
    static void __unpack( std::istream& s, unsigned& x )
      { s.read( (char *)&x, sizeof(unsigned) ); }
    static void __pack( std::ostream& s, unsigned x )
      { s.write( (const char *)&x, sizeof(unsigned) ); }
    static void __net_unpack( std::istream& s, long& x )
      {
        s.read( (char *)&x, sizeof(long) );
        x = EDS::from_net( x );
      }
    static void __net_pack( std::ostream& s, long x )
      {
        x = EDS::to_net( x );
        s.write( (const char *)&x, sizeof(long) );
      }
    static void __unpack( std::istream& s, long& x )
      { s.read( (char *)&x, sizeof(long) ); }
    static void __pack( std::ostream& s, long x )
      { s.write( (const char *)&x, sizeof(long) ); }
    static void __net_unpack( std::istream& s, unsigned long& x )
      {
        s.read( (char *)&x, sizeof(unsigned long) );
        x = EDS::from_net( x );
      }
    static void __net_pack( std::ostream& s, unsigned long x )
      {
        x = EDS::to_net( x );
        s.write( (const char *)&x, sizeof(unsigned long) );
      }
    static void __unpack( std::istream& s, unsigned long& x )
      { s.read( (char *)&x, sizeof(unsigned long) ); }
    static void __pack( std::ostream& s, unsigned long x )
      { s.write( (const char *)&x, sizeof(unsigned long) ); }
};

}

#endif // __EvPack_h

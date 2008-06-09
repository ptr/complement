// -*- C++ -*- Time-stamp: <08/06/09 21:59:13 yeti>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2008
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <sockios/netinfo.h>

#if defined(__unix) && !defined(__UCLIBC__) && !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
# include <stropts.h> // for ioctl() call
#endif

#include <fcntl.h>

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const char *name, int port,
                                            sock_base::stype type,
                                            sock_base::protocol prot )
{ return basic_sockbuf<charT, traits, _Alloc>::open( std::findhost( name ), port, type, prot ); }


template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( sock_base::socket_type s, sock_base::stype t )
{
  if ( basic_socket_t::is_open() || s == -1 ) {
    return 0;
  }

  sockaddr sa;
  socklen_t sl = sizeof(sa);
  getsockname( s, &sa, &sl );

  return basic_sockbuf<charT, traits, _Alloc>::open( s, sa, t );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::attach( sock_base::socket_type s,
                                              sock_base::stype t )
{
  if ( basic_socket_t::is_open() || s == -1 ) {
    return 0;
  }

  sockaddr sa;
  socklen_t sl = sizeof(sa);
  getsockname( s, &sa, &sl );

  return basic_sockbuf<charT, traits, _Alloc>::attach( s, sa, t );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::attach( sock_base::socket_type s,
                                              const sockaddr& addr,
                                              sock_base::stype t )
{
  if ( basic_socket_t::is_open() || s == -1 ) {
    return 0;
  }

  // _doclose = false;
  return basic_sockbuf<charT, traits, _Alloc>::open( dup(s), addr, t );
}

template<class charT, class traits, class _Alloc>
void basic_sockstream<charT, traits, _Alloc>::setoptions( sock_base::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  if ( _sb.is_open() ) {
    if ( optname != sock_base::so_linger ) {
      int turn = on_off ? 1 : 0;
      if ( setsockopt( _sb.fd(), SOL_SOCKET, (int)optname, (const void *)&turn,
                       (socklen_t)sizeof(int) ) != 0 ) {
        this->setstate( ios_base::failbit );
      }
    } else {
      linger l;
      l.l_onoff = on_off ? 1 : 0;
      l.l_linger = __v;
      if ( setsockopt( _sb.fd(), SOL_SOCKET, (int)optname, (const void *)&l,
                       (socklen_t)sizeof(linger) ) != 0 ) {
        this->setstate( ios_base::failbit );
      }
    }
  } else {
    this->setstate( ios_base::failbit );
  }
#endif // __unix
}

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif


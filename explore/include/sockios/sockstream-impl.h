// -*- C++ -*-

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2009, 2016
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <sockios/netinfo.h>
#include <fcntl.h>

#define __EXTRA_SOCK_OPT1
#define __EXTRA_SOCK_OPT2
#ifdef __FIT_SOCK_NONBLOCK
#  undef __EXTRA_SOCK_OPT1
#  define __EXTRA_SOCK_OPT1 | SOCK_NONBLOCK
#endif
#ifdef __FIT_SOCK_CLOEXEC
#  undef __EXTRA_SOCK_OPT2
#  define __EXTRA_SOCK_OPT2 | SOCK_CLOEXEC
#endif
#define __EXTRA_SOCK_OPT __EXTRA_SOCK_OPT1 __EXTRA_SOCK_OPT2

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

namespace detail {
extern unsigned local_mtu;
extern unsigned bt_max_frame;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc>*
basic_sockbuf<charT, traits, _Alloc>::open( const char* name, int port,
                                            sock_base::stype type,
                                            sock_base::protocol prot )
{
  return prot == sock_base::inet ?
    basic_sockbuf<charT, traits, _Alloc>::open( std::findhost( name ), port, type, prot ) :
    prot == sock_base::local ?
    basic_sockbuf<charT, traits, _Alloc>::open( name, type ) : 0;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc>*
basic_sockbuf<charT, traits, _Alloc>::open( const char* name, int port,
                                            const std::tr2::nanoseconds& timeout )
{
  return basic_sockbuf<charT, traits, _Alloc>::open( std::findhost( name ), port, timeout );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( in_addr_t addr, int port,
                                            sock_base::stype type,
                                            sock_base::protocol prot )
{
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _type = type;

    if ( prot == sock_base::inet ) {
      basic_socket_t::_fd = socket( PF_INET, type __EXTRA_SOCK_OPT, 0 );
      if ( basic_socket_t::_fd == -1 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
      basic_socket_t::_address.inet.sin_family = AF_INET;
      // htons is a define at least in Linux 2.2.5-15, and it's expantion fail
      // for gcc 2.95.3, so it not used here
#ifdef __LITTLE_ENDIAN
      basic_socket_t::_address.inet.sin_port = ((port >> 8) & 0xff) | ((port & 0xff) << 8);
      basic_socket_t::_address.inet.sin_addr.s_addr = ((addr >> 24) & 0xff) | ((addr & 0xff) << 24) | ((addr & 0xff00) << 8) | ((addr >> 8) & 0xff00);
#elif defined(__BIG_ENDIAN)
      basic_socket_t::_address.inet.sin_port = static_cast<in_port_t>(port);
      basic_socket_t::_address.inet.sin_addr.s_addr = addr;
#else
#  error Undefined byte order
#endif
  
      // Generally, stream sockets may successfully connect() only once
      if ( connect( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof( basic_socket_t::_address ) ) == -1 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
      if ( type == sock_base::sock_stream ) {
        _xwrite = &_Self_type::write;
        _xread = &_Self_type::read;
      } else if ( type == sock_base::sock_dgram ) {
        _xwrite = &_Self_type::sendto_in;
        _xread = &_Self_type::recvfrom_in;
      }
    } else if ( prot == sock_base::local ) {
      throw domain_error( "socket not belongs to inet type" );
    } else { // other protocols not implemented yet
      throw std::invalid_argument( "protocol not implemented" );
    }

    if ( _bbuf == 0 ) {
      _M_allocate_block( ((type == sock_base::sock_stream ? (basic_socket_t::default_mtu - 20 - 20) * 2 : (basic_socket_t::default_mtu - 20 - 8)) * 2) / sizeof(charT) );
    }

    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

#ifndef __FIT_SOCK_NONBLOCK
    if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
#endif
    this->setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );
    this->setg( this->epptr(), this->epptr(), this->epptr() );

    _fl = _fr = this->eback();

    basic_socket_t::mgr->push( *this );
  }
  catch ( std::system_error& ) {
    if ( basic_socket_t::_fd != -1 ) {
      ::close( basic_socket_t::_fd );
      basic_socket_t::_fd = -1;
    }
   
    return 0;
  }
  catch ( std::length_error& ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;
    return 0;
  }
  catch ( std::runtime_error& ) {
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( in_addr_t addr, int port, const std::tr2::nanoseconds& timeout )
{
  // open timeout for sock_base::sock_stream, sock_base::inet only
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _type = sock_base::sock_stream;

    basic_socket_t::_fd = socket( PF_INET, sock_base::sock_stream __EXTRA_SOCK_OPT, 0 );
    if ( basic_socket_t::_fd == -1 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }

#ifndef __FIT_SOCK_NONBLOCK
    if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
#endif

    basic_socket_t::_address.inet.sin_family = AF_INET;
    // htons is a define at least in Linux 2.2.5-15, and it's expantion fail
    // for gcc 2.95.3, so it not used here
#ifdef __LITTLE_ENDIAN
    basic_socket_t::_address.inet.sin_port = ((port >> 8) & 0xff) | ((port & 0xff) << 8);
    basic_socket_t::_address.inet.sin_addr.s_addr = ((addr >> 24) & 0xff) | ((addr & 0xff) << 24) | ((addr & 0xff00) << 8) | ((addr >> 8) & 0xff00);
#elif defined(__BIG_ENDIAN)
    basic_socket_t::_address.inet.sin_port = static_cast<in_port_t>(port);
    basic_socket_t::_address.inet.sin_addr.s_addr = addr;
#else
#  error Undefined byte order
#endif
  
    // Generally, stream sockets may successfully connect() only once
    if ( connect( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof( basic_socket_t::_address ) ) == -1 ) {
      if ( errno == EINPROGRESS ) {
        pollfd wpfd;
        wpfd.fd = basic_socket_t::_fd;
        wpfd.events = POLLOUT /* | POLLHUP | POLLWRNORM */
#if defined(_GNU_SOURCE)
                       | POLLRDHUP
#endif
          ;
        wpfd.revents = 0;
        while ( poll( &wpfd, 1, timeout.count() / 1000000LL ) <= 0 ) {
          // may be interrupted, check and ignore
          switch ( errno ) {
            case EINTR:
              errno = 0;
              continue;
            default:
              throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
          }
        }
        if ( (wpfd.revents & (POLLERR | POLLHUP
#if defined(_GNU_SOURCE)
                              | POLLRDHUP
#endif
                )) != 0 ) {
          throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
        }
      } else {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
    }

    _xwrite = &_Self_type::write;
    _xread = &_Self_type::read;

    if ( _bbuf == 0 ) {
      _M_allocate_block( (basic_socket_t::default_mtu - 20 - 20) * 2 );
    }

    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

    setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );
    setg( this->epptr(), this->epptr(), this->epptr() );

    _fl = _fr = this->eback();

    basic_socket_t::mgr->push( *this );
  }
  catch ( std::system_error& ) {
    if ( basic_socket_t::_fd != -1 ) {
      ::close( basic_socket_t::_fd );
      basic_socket_t::_fd = -1;
    }
   
    return 0;
  }
  catch ( std::length_error& ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;
    return 0;
  }
  catch ( std::runtime_error& ) {
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const char* path, sock_base::stype type )
{
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _type = type;
    basic_socket_t::_fd = socket( PF_UNIX, type __EXTRA_SOCK_OPT, 0 );
    if ( basic_socket_t::_fd == -1 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
    basic_socket_t::_address.unx.sun_family = AF_UNIX;
    strncpy(basic_socket_t::_address.unx.sun_path, path, sizeof(basic_socket_t::_address.unx.sun_path));

    // Generally, stream sockets may successfully connect() only once
    if ( connect( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof( basic_socket_t::_address.unx ) ) == -1 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
    if ( type == sock_base::sock_stream ) {
      _xwrite = &_Self_type::write;
      _xread = &_Self_type::read;
    } else if ( type == sock_base::sock_dgram ) {
      _xwrite = &_Self_type::sendto_un;
      _xread = &_Self_type::recvfrom_un;
    }

    if ( _bbuf == 0 ) {
      _M_allocate_block( (std::detail::local_mtu << 1) / sizeof(charT) );
    }

    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

#ifndef __FIT_SOCK_NONBLOCK
    if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
#endif
    this->setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );
    this->setg( this->epptr(), this->epptr(), this->epptr() );

    _fl = _fr = this->eback();

    basic_socket_t::mgr->push( *this );
  }
  catch ( std::system_error& ) {
    if ( basic_socket_t::_fd != -1 ) {
      ::close( basic_socket_t::_fd );
      basic_socket_t::_fd = -1;
    }
   
    return 0;
  }
  catch ( std::length_error& ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;
    return 0;
  }
  catch ( std::runtime_error& ) {
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const char* path, const std::tr2::nanoseconds& timeout, sock_base::stype type )
{
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _type = type;
    basic_socket_t::_fd = socket( PF_UNIX, type __EXTRA_SOCK_OPT, 0 );
    if ( basic_socket_t::_fd == -1 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
    basic_socket_t::_address.unx.sun_family = AF_UNIX;
    strncpy(basic_socket_t::_address.unx.sun_path, path, sizeof(basic_socket_t::_address.unx.sun_path));

#ifndef __FIT_SOCK_NONBLOCK
    if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
#endif

    // Generally, stream sockets may successfully connect() only once
    if ( connect( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof( basic_socket_t::_address.unx ) ) == -1 ) {
      if ( errno == EINPROGRESS ) {
        pollfd wpfd;
        wpfd.fd = basic_socket_t::_fd;
        wpfd.events = POLLOUT /* | POLLHUP | POLLWRNORM */
#if defined(_GNU_SOURCE)
                       | POLLRDHUP
#endif
          ;
        wpfd.revents = 0;
        while ( poll( &wpfd, 1, timeout.count() / 1000000LL ) <= 0 ) {
          // may be interrupted, check and ignore
          switch ( errno ) {
            case EINTR:
              errno = 0;
              continue;
            default:
              throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
          }
        }
        if ( (wpfd.revents & (POLLERR | POLLHUP
#if defined(_GNU_SOURCE)
                              | POLLRDHUP
#endif
                )) != 0 ) {
          throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
        }
      } else {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
    }
    if ( type == sock_base::sock_stream ) {
      _xwrite = &_Self_type::write;
      _xread = &_Self_type::read;
    } else if ( type == sock_base::sock_dgram ) {
      _xwrite = &_Self_type::sendto_un;
      _xread = &_Self_type::recvfrom_un;
    }

    if ( _bbuf == 0 ) {
      _M_allocate_block( (std::detail::local_mtu << 1) / sizeof(charT) );
    }

    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

    setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );
    setg( this->epptr(), this->epptr(), this->epptr() );

    _fl = _fr = this->eback();

    basic_socket_t::mgr->push( *this );
  }
  catch ( std::system_error& ) {
    if ( basic_socket_t::_fd != -1 ) {
      ::close( basic_socket_t::_fd );
      basic_socket_t::_fd = -1;
    }
   
    return 0;
  }
  catch ( std::length_error& ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;

    return 0;
  }
  catch ( std::runtime_error& ) {
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const sockaddr_in& addr,
                                            sock_base::stype type )
{
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _type = type;

    if ( addr.sin_family == AF_INET ) {
      basic_socket_t::_fd = socket( PF_INET, type __EXTRA_SOCK_OPT, 0 );
      if ( basic_socket_t::_fd == -1 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
      basic_socket_t::_address.inet = addr;
  
      // Generally, stream sockets may successfully connect() only once
      if ( connect( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof( basic_socket_t::_address ) ) == -1 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
      if ( type == sock_base::sock_stream ) {
        _xwrite = &_Self_type::write;
        _xread = &_Self_type::read;
      } else if ( type == sock_base::sock_dgram ) {
        _xwrite = &_Self_type::sendto_in;
        _xread = &_Self_type::recvfrom_in;
      }
    } else {
      throw domain_error( "socket not belongs to inet type" );
    }

    if ( _bbuf == 0 ) {
      _M_allocate_block( ((type == sock_base::sock_stream ? (basic_socket_t::default_mtu - 20 - 20) * 2 : (basic_socket_t::default_mtu - 20 - 8)) * 2) / sizeof(charT) );
    }

    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

#ifndef __FIT_SOCK_NONBLOCK
    if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
    }
#endif
    this->setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );
    this->setg( this->epptr(), this->epptr(), this->epptr() );

    _fl = _fr = this->eback();

    basic_socket_t::mgr->push( *this );
  }
  catch ( std::system_error& ) {
    if ( basic_socket_t::_fd != -1 ) {
      ::close( basic_socket_t::_fd );
      basic_socket_t::_fd = -1;
    }
   
    return 0;
  }
  catch ( std::length_error& ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;
    return 0;
  }
  catch ( std::runtime_error& ) {
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( const sockaddr_in& addr,
                                            const std::tr2::nanoseconds& timeout,
                                            sock_base::stype type )
{
  if ( is_open() ) {
    return 0;
  }
  try {
    _mode = ios_base::in | ios_base::out;
    _type = type;

    if ( addr.sin_family == AF_INET ) {
      basic_socket_t::_fd = socket( PF_INET, type __EXTRA_SOCK_OPT, 0 );
      if ( basic_socket_t::_fd == -1 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
      basic_socket_t::_address.inet = addr;

#ifndef __FIT_SOCK_NONBLOCK
      if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
        throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
      }
#endif

      // Generally, stream sockets may successfully connect() only once
      if ( connect( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof( basic_socket_t::_address ) ) == -1 ) {
        if ( errno == EINPROGRESS ) {
          pollfd wpfd;
          wpfd.fd = basic_socket_t::_fd;
          wpfd.events = POLLOUT /* | POLLHUP | POLLWRNORM */
#if defined(_GNU_SOURCE)
                        | POLLRDHUP
#endif
            ;
          wpfd.revents = 0;
          while ( poll( &wpfd, 1, timeout.count() / 1000000LL ) <= 0 ) {
            // may be interrupted, check and ignore
            switch ( errno ) {
              case EINTR:
                errno = 0;
                continue;
              default:
                throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
            }
          }
          if ( (wpfd.revents & (POLLERR | POLLHUP
#if defined(_GNU_SOURCE)
                                | POLLRDHUP
#endif
                  )) != 0 ) {
            throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
          }
        } else {
          throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>::open" ) );
        }
      }
      if ( type == sock_base::sock_stream ) {
        _xwrite = &_Self_type::write;
        _xread = &_Self_type::read;
      } else if ( type == sock_base::sock_dgram ) {
        _xwrite = &_Self_type::sendto_in;
        _xread = &_Self_type::recvfrom_in;
      }
    } else {
      throw domain_error( "socket not belongs to inet type" );
    }

    if ( _bbuf == 0 ) {
      _M_allocate_block( ((type == sock_base::sock_stream ? (basic_socket_t::default_mtu - 20 - 20) * 2 : (basic_socket_t::default_mtu - 20 - 8)) * 2) / sizeof(charT) );
    }

    if ( _bbuf == 0 ) {
      throw std::length_error( "can't allocate block" );
    }

    setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );
    setg( this->epptr(), this->epptr(), this->epptr() );

    _fl = _fr = this->eback();

    basic_socket_t::mgr->push( *this );
  }
  catch ( std::system_error& ) {
    if ( basic_socket_t::_fd != -1 ) {
      ::close( basic_socket_t::_fd );
      basic_socket_t::_fd = -1;
    }
   
    // std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );
    // ucnd.notify_all();

    return 0;
  }
  catch ( std::length_error& ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;

    return 0;
  }
  catch ( std::runtime_error& ) {
    return 0;
  }
  catch ( std::invalid_argument& ) {
    return 0;
  }

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( sock_base::socket_type s, sock_base::stype t )
{
  if ( is_open() || s == -1 ) {
    return 0;
  }

  sockaddr sa;
  socklen_t sl = sizeof(sa);
  getsockname( s, &sa, &sl );

  return basic_sockbuf<charT, traits, _Alloc>::open( s, sa, t );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::open( sock_base::socket_type s,
                                            const sockaddr& addr,
                                            sock_base::stype t )
{
  basic_sockbuf<charT, traits, _Alloc>* ret = _open_sockmgr( s, addr, t );
  if ( (ret != 0) && (t == sock_base::sock_stream) ) {
    // push to mgr only statefull (connected) sockets
    basic_socket_t::mgr->push( *this );
  }
  return ret;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::attach( sock_base::socket_type s,
                                              sock_base::stype t )
{
  if ( s == -1 ) {
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
  if ( s == -1 ) {
    return 0;
  }

  if ( is_open() ) {
    std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );

    if ( (s != basic_socket_t::_fd) || (t != _type) ||
         (basic_socket_t::_address.any.sa_family != addr.sa_family) ) {
      return 0;
    }
    memcpy( (void *)&basic_socket_t::_address.any, (const void *)&addr, sizeof(sockaddr) );
    return this;
  }

  return basic_sockbuf<charT, traits, _Alloc>::open( dup(s), addr, t );
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::_open_sockmgr( sock_base::socket_type s,
                                                     const sockaddr& addr,
                                                     sock_base::stype t )
{
  std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );

  if ( basic_socket_t::is_open_unsafe() || s == -1 ) {
    return 0;
  }

  basic_socket_t::_fd = s;
  memcpy( (void *)&basic_socket_t::_address.any, (const void *)&addr, sizeof(sockaddr) );
  _mode = ios_base::in | ios_base::out;
  _type = t;

  if ( t == sock_base::sock_stream ) {
    _xwrite = &_Self_type::write;
    _xread = &_Self_type::read;
  } else if ( t == sock_base::sock_dgram ) {
    if ( basic_socket_t::_address.any.sa_family == AF_INET ) {
      _xwrite = &_Self_type::sendto_in;
      _xread = &_Self_type::recvfrom_in;
    } else if ( basic_socket_t::_address.any.sa_family == AF_UNIX ) {
      _xwrite = &_Self_type::sendto_un;
      _xread = &_Self_type::recvfrom_un;
    } else {
    }
  } else if ( t == sock_base::sock_raw ) {
    if ( basic_socket_t::_address.any.sa_family == AF_INET ) {
      _xwrite = &_Self_type::sendto_in;
      _xread = &_Self_type::recvfrom_in;
    } else if ( basic_socket_t::_address.any.sa_family == AF_UNIX ) {
      _xwrite = &_Self_type::sendto_un;
      _xread = &_Self_type::recvfrom_un;
    } else if ( basic_socket_t::_address.any.sa_family == AF_BLUETOOTH ) {
      _xwrite = &_Self_type::write;
      _xread = &_Self_type::read;
    }
  } else {
    basic_socket_t::_fd = -1;
    return 0; // unsupported type
  }

  if ( _bbuf == 0 ) {
    if ( basic_socket_t::_address.any.sa_family == AF_INET ) {
      _M_allocate_block( ((t == sock_base::sock_stream ? (basic_socket_t::default_mtu - 20 - 20) * 2 : (basic_socket_t::default_mtu - 20 - 8)) * 2) / sizeof(charT) );
    } else if ( basic_socket_t::_address.any.sa_family == AF_UNIX ) {
      _M_allocate_block( (std::detail::local_mtu << 1) / sizeof(charT) );
    } else if ( basic_socket_t::_address.any.sa_family == AF_BLUETOOTH ) {
      _M_allocate_block( (std::detail::bt_max_frame << 1) / sizeof(charT) );
    }
  }

  if ( _bbuf == 0 ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;

    return 0;
  }

  // I don't know, how _fd was created, so not used __FIT_SOCK_NONBLOCK here,
  // just try to set O_NONBLOCK...
  if ( fcntl( basic_socket_t::_fd, F_SETFL, fcntl( basic_socket_t::_fd, F_GETFL ) | O_NONBLOCK ) != 0 ) {
    ::close( basic_socket_t::_fd );
    basic_socket_t::_fd = -1;
    throw std::system_error( errno, std::get_posix_category(), std::string( "basic_sockbuf<charT, traits, _Alloc>" ) );
  }
  this->setp( _bbuf, _bbuf + ((_ebuf - _bbuf)>>1) );

  this->setg( this->epptr(), this->epptr(), this->epptr() );

  _fl = _fr = this->eback();

  return this;
}

template<class charT, class traits, class _Alloc>
basic_sockbuf<charT, traits, _Alloc> *
basic_sockbuf<charT, traits, _Alloc>::close()
{
  std::tr2::unique_lock<std::tr2::recursive_mutex> lk( ulck );

  if ( !basic_socket_t::is_open_unsafe() ) {
    return 0;
  }

  shutdown_unsafe( sock_base::stop_in | sock_base::stop_out );
  rewind();

  ucnd.wait( lk, closed_t( *this ) );

  return this;
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::shutdown_unsafe( sock_base::shutdownflg dir )
{
  if ( basic_socket_t::is_open_unsafe() ) {
    if ( (dir & (sock_base::stop_in | sock_base::stop_out)) ==
         (sock_base::stop_in | sock_base::stop_out) ) {
      ::shutdown( basic_socket_t::_fd, 2 );
    } else if ( dir & sock_base::stop_in ) {
      ::shutdown( basic_socket_t::_fd, 0 );
    } else if ( dir & sock_base::stop_out ) {
      ::shutdown( basic_socket_t::_fd, 1 );
    }
  }
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::rewind()
{
  if ( this->gptr() == this->_ebuf ) {
    this->_fr = this->_fl;
    this->_fl = this->eback();
    this->setg( this->eback(), this->eback(), this->_fr );
  } else if ( this->_fr != this->egptr() ) {
    this->setg( this->eback(), this->gptr(), this->_fr );
  }

  if ( this->is_open_unsafe() && ((this->_fr < this->_ebuf) || (this->_fl < this->gptr())) ) {
    // restore descriptor in epoll vector, free space in buffer available
    basic_socket_t::mgr->epoll_restore( this->fd_unsafe() );
  }
}

template<class charT, class traits, class _Alloc>
__FIT_TYPENAME basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::underflow()
{
  std::tr2::unique_lock<std::tr2::recursive_mutex> lk( ulck );

  rewind();

  if ( this->gptr() < this->egptr() ) {
    return traits::to_int_type(*this->gptr());
  }

  // no ready data, wait on condition
  if ( basic_socket_t::_use_rdtimeout ) {
    if ( !ucnd.timed_wait( lk, basic_socket_t::_rdtimeout, rdready ) ) {
      return traits::eof();
    }
  } else {
    ucnd.wait( lk, rdready );
  }

  rewind();

  if ( this->gptr() < this->egptr() ) {
    return traits::to_int_type(*this->gptr());
  }

  return traits::eof();
}

template<class charT, class traits, class _Alloc>
__FIT_TYPENAME basic_sockbuf<charT, traits, _Alloc>::int_type
basic_sockbuf<charT, traits, _Alloc>::overflow( int_type c )
{
  if ( !is_open() ) {
    return traits::eof();
  }

  if ( !traits::eq_int_type( c, traits::eof() ) && this->pptr() < this->epptr() ) {
    this->sputc( traits::to_char_type(c) );
    return c;
  }

  long count = this->pptr() - this->pbase();

  if ( count ) {
    count *= sizeof(charT);
    long offset;
    do {
      offset = (this->*_xwrite)( this->pbase(), count );
      if ( offset < 0 ) {
        switch ( errno ) {
          case EINTR:
            errno = 0;
            break;
          case EAGAIN:
            errno = 0;
            {
              pollfd wpfd;
              wpfd.fd = basic_socket_t::_fd;
              wpfd.events = POLLOUT /* | POLLHUP | POLLWRNORM */
#if defined(_GNU_SOURCE)
                     | POLLRDHUP
#endif
                         ;
              wpfd.revents = 0;
              while ( poll( &wpfd, 1, basic_socket_t::_use_wrtimeout ? basic_socket_t::_wrtimeout.count() / 1000000LL: -1 ) <= 0 ) { // wait infinite
                // may be interrupted, check and ignore
                switch ( errno ) {
                  case EINTR:
                    errno = 0;
                    continue;
                  default:
                    return traits::eof();
                }
              }
              if ( (wpfd.revents & (POLLERR | POLLHUP
#if defined(_GNU_SOURCE)
                                    | POLLRDHUP
#endif
                      )) != 0 ) {
                return traits::eof();
              }
            }
            break;
          default:
            return traits::eof();
        }
      }
    } while ( offset < 0 );

    if ( offset < count ) {
      // MUST BE: (offset % sizeof(char_traits)) == 0 !
      offset /= sizeof(charT);
      count /= sizeof(charT);
      traits::move( this->pbase(), this->pbase() + offset, count - offset );
      // std::copy_backword( this->pbase() + offset, this->pbase() + count, this->pbase() );
      this->setp( this->pbase(), this->epptr() ); // require: set pptr
      this->pbump( count - offset );
      if( !traits::eq_int_type(c,traits::eof()) ) {
        this->sputc( traits::to_char_type(c) );
      }

      return traits::not_eof(c);
    }
  }

  this->setp( this->pbase(), this->epptr() ); // require: set pptr
  if( !traits::eq_int_type(c,traits::eof()) ) {
    this->sputc( traits::to_char_type(c) );
  }

  return traits::not_eof(c);
}

template<class charT, class traits, class _Alloc>
int basic_sockbuf<charT, traits, _Alloc>::sync()
{
  if ( !is_open() ) {
    return -1;
  }

  long count = this->pptr() - this->pbase();
  if ( count ) {
    count *= sizeof(charT);
    long start = 0L;
    long offset = 0L;
    while ( count > 0 ) {
      do {
        offset = (this->*_xwrite)( this->pbase() + start, count );
        if ( offset < 0 ) {
          switch ( errno ) {
            case EINTR:
              errno = 0;
              break;
            case EAGAIN:
              errno = 0;
              {
                pollfd wpfd;
                wpfd.fd = basic_socket_t::_fd;
                wpfd.events = POLLOUT /* | POLLHUP | POLLWRNORM */
#if defined(_GNU_SOURCE)
                     | POLLRDHUP
#endif
                  ;
                wpfd.revents = 0;
                while ( poll( &wpfd, 1, basic_socket_t::_use_wrtimeout ? basic_socket_t::_wrtimeout.count() / 1000000LL : -1 ) <= 0 ) { // wait infinite
                  // may be interrupted, check and ignore
                  switch ( errno ) {
                    case EINTR:
                      errno = 0;
                      continue;
                    default:
                      return -1;
                  }
                }
                if ( (wpfd.revents & (POLLERR | POLLHUP
#if defined(_GNU_SOURCE)
                                      | POLLRDHUP
#endif
                        )) != 0 ) {
                  return -1;
                }
              }
              break;
            default:
              return -1;
          }
        }
      } while ( offset < 0 );

      count -= offset;
      start += offset;
    }
    this->setp( this->pbase(), this->epptr() ); // require: set pptr
  }

  return 0;
}

template<class charT, class traits, class _Alloc>
streamsize basic_sockbuf<charT, traits, _Alloc>::xsputn( const char_type *s, streamsize n )
{
  if ( !is_open() || s == 0 || n == 0 ) {
    return 0;
  }

  streamsize __n_put = this->epptr() - this->pptr();

  if ( __n_put >= n ) {
    traits::copy( this->pptr(), s, n );
    this->pbump( n );
    return n;
  }

  if ( __n_put == 0 ) {
    if ( traits::eq_int_type(overflow(),traits::eof()) ) {
      return 0;
    }
    __n_put = this->epptr() - this->pptr();
  }

  streamsize count = 0;

  while ( __n_put < n ) { // __n_put > 0 here
    traits::copy( this->pptr(), s, __n_put );
    this->pbump( __n_put );
    n -= __n_put;
    s += __n_put;
    count += __n_put;
    if ( traits::eq_int_type(overflow(),traits::eof()) ) {
      return count;
    }
    __n_put = this->epptr() - this->pptr();
  }

  traits::copy( this->pptr(), s, n );
  this->pbump( n );
  count += n;

  return count;
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::setoptions( sock_base::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );

  if ( basic_socket_t::is_open_unsafe() ) {
    int turn = on_off ? 1 : 0;
    int ret = 0;
    switch ( optname ) {
      case sock_base::so_debug:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_DEBUG, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_keepalive:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_KEEPALIVE, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_linger:
        {
          linger l;
          l.l_onoff = on_off ? 1 : 0;
          l.l_linger = __v;
          ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_LINGER, (const void *)&l, (socklen_t)sizeof(linger) );
        }
        break;
      case sock_base::so_oobinline:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_OOBINLINE, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_nodelay:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_cork:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_CORK, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_quickack:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_QUICKACK, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
#ifdef TCP_CONGESTION
      case sock_base::so_tcp_congestion:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_CONGESTION, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
#endif
      default:
        throw std::invalid_argument( "bad socket option" );
    }
    if ( ret != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "socket option" ) );
    }
  } else {
    throw std::invalid_argument( "socket is closed" );
  }
#endif // __unix
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::setoptions( sock_base::so_t optname, int __v )
{
#ifdef __unix
  std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );

  if ( basic_socket_t::is_open_unsafe() ) {
    int ret = 0;
    switch ( optname ) {
      case sock_base::so_sndbuf:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_SNDBUF, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_rcvbuf:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_RCVBUF, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_sndlowat:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_SNDLOWAT, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_rcvlowat:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_RCVLOWAT, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_sndtimeo:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_SNDTIMEO, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_rcvtimeo:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_maxseg:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_MAXSEG, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_keepidle:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_KEEPIDLE, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_keepintvl:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_KEEPINTVL, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_keepcnt:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_KEEPCNT, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_syncnt:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_SYNCNT, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_linger2:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_LINGER2, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      case sock_base::so_tcp_window_clamp:
        if ( _type != sock_base::sock_stream ) {
          throw std::invalid_argument( "bad socket option" );
        }
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_WINDOW_CLAMP, (const void *)&__v, (socklen_t)sizeof(int) );
        break;
      default:
        throw std::invalid_argument( "bad socket option" );
    }
    if ( ret != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "socket option" ) );
    }
  } else {
    throw std::invalid_argument( "socket is closed" );
  }
#endif // __unix
}

template<class charT, class traits, class _Alloc>
void basic_sockbuf<charT, traits, _Alloc>::setoptions( const bt::hci::sock_filter& f )
{
  std::tr2::lock_guard<std::tr2::recursive_mutex> lk( ulck );

  if ( basic_socket_t::is_open_unsafe() ) {
    int ret = 0;
    ret = setsockopt( basic_socket_t::_fd, SOL_HCI, bt::hci::sock_filter::SO_HCI_FILTER, (const void *)&f, (socklen_t)sizeof(bt::hci::sock_filter) );
    if ( ret != 0 ) {
      throw std::system_error( errno, std::get_posix_category(), std::string( "socket option" ) );
    }
  } else {
    throw std::invalid_argument( "socket is closed" );
  }
}

#undef __EXTRA_SOCK_OPT
#undef __EXTRA_SOCK_OPT1
#undef __EXTRA_SOCK_OPT2

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif


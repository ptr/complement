// -*- C++ -*-

/*
 * Copyright (c) 2008-2010, 2016, 2017, 2019
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/defs.h>

#define __EXTRA_SOCK_OPT1
#ifdef __FIT_SOCK_CLOEXEC
#  undef __EXTRA_SOCK_OPT1
#  define __EXTRA_SOCK_OPT1 | SOCK_CLOEXEC
#endif
#define __EXTRA_SOCK_OPT __EXTRA_SOCK_OPT1

namespace std {

namespace detail {
extern std::tr2::mutex _se_lock;
extern std::ostream* _se_stream;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( const in_addr& addr, int port, sock_base::stype type, sock_base::protocol prot )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _type = type;

  if ( prot == sock_base::inet ) {
    basic_socket_t::_fd = socket( PF_INET, type __EXTRA_SOCK_OPT, 0 );
    if ( basic_socket_t::_fd == -1 ) {
      _state |= ios_base::failbit | ios_base::badbit;
      return;
    }
    // _open = true;
    basic_socket_t::_address.inet.sin_family = AF_INET;
    basic_socket_t::_address.inet.sin_port = htons( port );
    basic_socket_t::_address.inet.sin_addr.s_addr = addr.s_addr;

    if ( type == sock_base::sock_stream || type == sock_base::sock_seqpacket ) {
      // let's try reuse local address
      setoptions_unsafe( sock_base::so_reuseaddr, true );
    }

    if ( ::bind( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof(basic_socket_t::_address) ) == -1 ) {
      _state |= ios_base::failbit;
      ::close( basic_socket_t::_fd );
      basic_socket_t::_fd = -1;
      return;
    }

    if ( type == sock_base::sock_stream || type == sock_base::sock_seqpacket ) {
      // I am shure, this is socket of type SOCK_STREAM | SOCK_SEQPACKET,
      // so don't check return code from listen
      ::listen( basic_socket_t::_fd, SOMAXCONN );
      basic_socket_t::mgr->push( *this );
    }
  } else {
    throw domain_error( "socket not belongs to inet type" );
  }

  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( const char* path, sock_base::stype type )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _type = type;

  basic_socket_t::_fd = socket( PF_UNIX, type __EXTRA_SOCK_OPT, 0 );
  if ( basic_socket_t::_fd == -1 ) {
    _state |= ios_base::failbit | ios_base::badbit;
    return;
  }
  basic_socket_t::_address.unx.sun_family = AF_UNIX;
  strncpy(basic_socket_t::_address.unx.sun_path, path, sizeof(basic_socket_t::_address.unx.sun_path));
  unlink( path ); // ignore error

  if ( ::bind( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof(basic_socket_t::_address.unx.sun_family) + strlen(basic_socket_t::_address.unx.sun_path) ) == -1 ) {
    _state |= ios_base::failbit;
    ::close( basic_socket_t::_fd );

    basic_socket_t::_fd = -1;
    return;
  }
  if ( type == sock_base::sock_dgram ) {
    // Concept only, I lost sockstream* here:
    // create_stream( basic_socket_t::_fd, basic_socket_t::_address.any );
    basic_socket_t::mgr->push_dp( *this );
  } else if ( type == sock_base::sock_stream ) {
    // I am shure, this is socket of type SOCK_STREAM,
    // so don't check return code from listen
    ::listen( basic_socket_t::_fd, SOMAXCONN );
    basic_socket_t::mgr->push( *this );
  }

  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( int dev, int channel, sock_base::protocol prot, const adopt_bt_t& )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _type = std::sock_base::sock_raw; // ? used for bluetooth socket

  switch (prot) {
    case sock_base::bt_hci:
      basic_socket_t::_fd = socket(PF_BLUETOOTH, sock_base::sock_raw __EXTRA_SOCK_OPT, BTPROTO_HCI);

      if ( basic_socket_t::_fd == -1 ) {
        _state |= ios_base::failbit | ios_base::badbit;
        return;
      }
      basic_socket_t::_address.bt_hci.sbt_hci_family = AF_BLUETOOTH;
      basic_socket_t::_address.bt_hci.dev = dev;
      basic_socket_t::_address.bt_hci.channel = channel;

      if ( ::bind( basic_socket_t::_fd, &basic_socket_t::_address.any, sizeof(basic_socket_t::_address) ) == -1 ) {
        _state |= ios_base::failbit;
        ::close( basic_socket_t::_fd );

        basic_socket_t::_fd = -1;
        return;
      } else {
        // immediately set filter "allow all", otherwise no data come in
        bt::hci::sock_filter f;

        f.clear();
        f.all_ptypes();
        f.all_events();
        // f.opcode(htobs(cmd_opcode_pack(1, 1)));
        f.opcode(0);

        int ret = setsockopt( basic_socket_t::_fd, SOL_HCI, bt::hci::sock_filter::SO_HCI_FILTER, (const void *)&f, (socklen_t)sizeof(bt::hci::sock_filter) );
        if (ret) {
          _state |= ios_base::failbit;
          ::close( basic_socket_t::_fd );
          basic_socket_t::_fd = -1;
          return;
        }
      }

      break;

    default:
      _state |= ios_base::failbit | ios_base::badbit;
      return;
  }

  basic_socket_t::mgr->push_dp( *this );

  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( const char *path, const adopt_dev_t& )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _type = std::sock_base::sock_raw; // ? used for bluetooth socket

  basic_socket_t::_fd = ::open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC);

  if ( basic_socket_t::_fd == -1 ) {
    _state |= ios_base::failbit | ios_base::badbit;
    return;
  }
  basic_socket_t::_address.any.sa_family = AF_UNSPEC;

  basic_socket_t::mgr->push_nsp( *this );

  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( int fd, const adopt_dev_t& )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _type = std::sock_base::sock_raw; // ? used for bluetooth socket

  basic_socket_t::_fd = fd;

  if ( basic_socket_t::_fd == -1 ) {
    _state |= ios_base::failbit | ios_base::badbit;
    return;
  }
  basic_socket_t::_address.any.sa_family = AF_UNSPEC;

  basic_socket_t::mgr->push_nsp( *this );

  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( const char *path, const adopt_tty_t& )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _type = std::sock_base::tty;

  basic_socket_t::_fd = ::open(path, O_RDWR | O_NONBLOCK | O_CLOEXEC | O_NOCTTY);

  if ( basic_socket_t::_fd == -1 ) {
    _state |= ios_base::failbit | ios_base::badbit;
    return;
  }
  tcgetattr(basic_socket_t::_fd, &basic_socket_t::_address.term);

  basic_socket_t::mgr->push_nsp( *this );

  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::open( int fd, const adopt_tty_t& )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    return;
  }
  _mode = ios_base::in | ios_base::out;
  _state = ios_base::goodbit;
  _type = std::sock_base::tty;

  basic_socket_t::_fd = fd;

  if ( basic_socket_t::_fd == -1 ) {
    _state |= ios_base::failbit | ios_base::badbit;
    return;
  }
  tcgetattr(basic_socket_t::_fd, &basic_socket_t::_address.term);

  basic_socket_t::mgr->push_nsp( *this );

  _state = ios_base::goodbit;

  return;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::_close()
{
  if ( !is_open() ) {
    return;
  }

  std::tr2::unique_lock<std::tr2::mutex> clk(_cnt_lck);
  
  /* all command in pipe should be processed:
     otherwise shutdown signal to descriptor, that may not
     in epoll vector yet
   */
  _cnt_cnd.wait( clk, _chk );

  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);

  if (_type != std::sock_base::tty) {
    ::shutdown( basic_socket_t::_fd, 0 );
  } else {
    ::tcflush(basic_socket_t::_fd, TCIFLUSH);
  }

  basic_socket_t::_fd = -1;

  _state |= ios_base::eofbit;
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::shutdown( sock_base::shutdownflg dir )
{
  std::tr2::lock_guard<std::tr2::mutex> lk(_fd_lck);
  if ( basic_socket_t::is_open_unsafe() ) {
    if ( (dir & (sock_base::stop_in | sock_base::stop_out)) ==
         (sock_base::stop_in | sock_base::stop_out) ) {
      if ( _type != sock_base::tty ) {
        ::shutdown( basic_socket_t::_fd, 2 );
      } else {
        ::tcflush(basic_socket_t::_fd, TCIOFLUSH);
      }
    } else if ( dir & sock_base::stop_in ) {
      if ( _type != sock_base::tty ) {
        ::shutdown( basic_socket_t::_fd, 0 );
      } else {
        ::tcflush(basic_socket_t::_fd, TCIFLUSH);
      }
    } else if ( dir & sock_base::stop_out ) {
      if ( _type != sock_base::tty ) {
        ::shutdown( basic_socket_t::_fd, 1 );
      } else {
        ::tcflush(basic_socket_t::_fd, TCOFLUSH);
      }
    }
  }
}

template<class charT, class traits, class _Alloc>
void sock_processor_base<charT,traits,_Alloc>::setoptions_unsafe( sock_base::so_t optname, bool on_off, int __v )
{
#ifdef __unix
  if ( basic_socket_t::is_open_unsafe() ) {
    int turn = on_off ? 1 : 0;
    int ret = 0;
    switch ( optname ) {
      case so_debug:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_DEBUG, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case so_acceptconn:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_ACCEPTCONN, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case so_reuseaddr:
        ret = setsockopt( basic_socket_t::_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      case so_tcp_defer_accept:
        ret = setsockopt( basic_socket_t::_fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, (const void *)&turn, (socklen_t)sizeof(int) );
        break;
      default:
        throw std::invalid_argument( "bad socket option" );
    }
    if ( ret != 0 ) {
      throw system_error( errno, system_category(), std::string( __PRETTY_FUNCTION__ ) );
    }
  } else {
    throw std::invalid_argument( "socket is closed" );
  }
#endif // __unix
}


template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
int connect_processor<Connect, charT, traits, _Alloc, C>::Init::_count = 0;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
bool connect_processor<Connect, charT, traits, _Alloc, C>::Init::_at_fork = false;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
std::tr2::mutex connect_processor<Connect, charT, traits, _Alloc, C>::Init::_init_lock;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::_guard( int direction )
{
  if ( direction ) {
    std::tr2::lock_guard<std::tr2::mutex> lk( _init_lock );
    if ( _count++ == 0 ) {
#ifdef __FIT_PTHREADS
      if ( !_at_fork ) { // call only once
        if ( pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child ) ) {
          // throw system_error
        }
        _at_fork = true;
      }
#endif
    }
  } else {
    std::tr2::lock_guard<std::tr2::mutex> lk( _init_lock );
    if ( --_count == 0 ) {

    }
  }
}

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_prepare()
{ _init_lock.lock(); }

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_child()
{
  _init_lock.unlock();

  if ( _count != 0 ) {
    throw std::logic_error( "Fork while connect_processor working may has unexpected behaviour in child process" );
  }
}

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::Init::__at_fork_parent()
{ _init_lock.unlock(); }

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
char connect_processor<Connect, charT, traits, _Alloc, C>::Init_buf[128];

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
typename connect_processor<Connect, charT, traits, _Alloc, C>::base_t::sockbuf_t* connect_processor<Connect, charT, traits, _Alloc, C>::operator()( sock_base::socket_type fd, const sockaddr& addr )
{
  processor p;
  p.s = base_t::create_stream( fd, addr );

  if ( p.s == 0 ) {
    // 0 is dangerous as return value and is unexpected,
    // because the object by this pointer may be accessed
    // later in some cases
    // misc::use_syslog<LOG_DEBUG>() << HERE << ":unexpected" << endl;
// #ifdef __FIT_STEM_TRACE
    try {
      std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
        ios_base::fmtflags f = _trs->flags( ios_base::showbase );
        *_trs << HERE << ":unexpected";
#ifdef STLPORT
        _trs->flags( f );
#else
        _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
// #endif // __FIT_STEM_TRACE

    return 0;
  }

  {
    std::tr2::lock_guard<std::tr2::mutex> lk( ready_lock );
    ready_queue.push( request_t( socket_open, fd, p ) );
    ready_cnd.notify_one();
  }

  return p.s->rdbuf();
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( sock_base::socket_type fd )
{
  std::tr2::lock_guard<std::tr2::mutex> lk( ready_lock );
  ready_queue.push( request_t( socket_read, fd ) );
  ready_cnd.notify_one();
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::operator ()( sock_base::socket_type fd, const typename base_t::adopt_close_t& )
{
  std::tr2::lock_guard<std::tr2::mutex> lk( ready_lock );
  ready_queue.push( request_t( socket_close, fd ) );
  ready_cnd.notify_one();
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::feed_data_to_processor( processor& p )
{
  try {
    for ( typename at_container_type::const_iterator i = _at_data.begin(); i != _at_data.end(); ++i ) {
      (*i)( *p.s );
    }

    while ( p.s->rdbuf()->is_ready() && p.s->good() ) { 
      (p.c->*C)( *p.s );
    }

    if ( p.s->is_open() ) {
      p.s->rdbuf()->pubrewind(); // worry about free space in income buffer
    }
  }
  catch ( const std::exception& e ) {
    // misc::use_syslog<LOG_DEBUG>() << HERE << ":exception from Connect::ctor " << e.what() << endl;
// #ifdef __FIT_STEM_TRACE
    try {
      std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
        ios_base::fmtflags f = _trs->flags( ios_base::showbase );
        *_trs << HERE << ":exception from Connect::ctor " << e.what() << endl;
#ifdef STLPORT
        _trs->flags( f );
#else
        _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
// #endif // __FIT_STEM_TRACE
  }
  catch ( ... ) {
    // misc::use_syslog<LOG_DEBUG>() << HERE << ":exception from Connect::ctor unknown" << endl;
// #ifdef __FIT_STEM_TRACE
    try {
      std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
        ios_base::fmtflags f = _trs->flags( ios_base::showbase );
        *_trs << HERE << ":exception from Connect::ctor unknown" << endl;
#ifdef STLPORT
        _trs->flags( f );
#else
        _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
// #endif // __FIT_STEM_TRACE
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::process_request( const request_t& request )
{
  try {
    if ( request.operation_type == socket_open ) {
      typename opened_pool_t::iterator opened_processor_iterator = opened_pool.find( request.fd );

      if ( opened_processor_iterator != opened_pool.end() ) {
        // misc::use_syslog<LOG_DEBUG>() << HERE << ":unexpected" << endl;
// #ifdef __FIT_STEM_TRACE
        try {
          std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
            ios_base::fmtflags f = _trs->flags( ios_base::showbase );
            *_trs << HERE << ":unexpected" << endl;
#ifdef STLPORT
            _trs->flags( f );
#else
            _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
          }
        }
        catch ( ... ) {
        }
// #endif // __FIT_STEM_TRACE
        return;
      }

      if ( request.p.s == 0 ) {
        // misc::use_syslog<LOG_DEBUG>() << HERE << ":unexpected" << endl;
// #ifdef __FIT_STEM_TRACE
        try {
          std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
            ios_base::fmtflags f = _trs->flags( ios_base::showbase );
            *_trs << HERE << ":unexpected" << endl;
#ifdef STLPORT
            _trs->flags( f );
#else
            _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
          }
        }
        catch ( ... ) {
        }
// #endif // __FIT_STEM_TRACE
        return;
      }

      for ( typename at_container_type::const_iterator i = _at_connect.begin(); i != _at_connect.end(); ++i ) {
        (*i)( *request.p.s );
      }

      processor p;
      p.s = request.p.s;

      try {
        p.c = new Connect( *p.s ); 
      }
      catch ( const std::exception& e ) {
        // misc::use_syslog<LOG_DEBUG>() << HERE << ":exception from Connect::ctor " << e.what() << endl;
 // #ifdef __FIT_STEM_TRACE
        try {
          std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
            ios_base::fmtflags f = _trs->flags( ios_base::showbase );
            *_trs << HERE << ":exception from Connect::ctor " << e.what() << endl;
#ifdef STLPORT
            _trs->flags( f );
#else
            _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
          }
        }
        catch ( ... ) {
        }
// #endif // __FIT_STEM_TRACE
       return;
      }
      catch ( ... ) {
        // misc::use_syslog<LOG_DEBUG>() << HERE << ":exception from Connect::ctor unknown" << endl;
// #ifdef __FIT_STEM_TRACE
        try {
          std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
            ios_base::fmtflags f = _trs->flags( ios_base::showbase );
            *_trs << HERE << ":exception from Connect::ctor unknown" << endl;
#ifdef STLPORT
            _trs->flags( f );
#else
            _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
          }
        }
        catch ( ... ) {
        }
// #endif // __FIT_STEM_TRACE
        return;
      }

      opened_pool[request.fd] = p;

      feed_data_to_processor( p );
    } else if ( request.operation_type == socket_read ) {
      typename opened_pool_t::iterator opened_processor_iterator = opened_pool.find( request.fd );

      if ( opened_processor_iterator == opened_pool.end() ) {
        // misc::use_syslog<LOG_DEBUG>() << HERE << ":unexpected" << endl;
// #ifdef __FIT_STEM_TRACE
        try {
          std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
            ios_base::fmtflags f = _trs->flags( ios_base::showbase );
            *_trs << HERE << ":unexpected" << endl;
#ifdef STLPORT
            _trs->flags( f );
#else
            _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
          }
        }
        catch ( ... ) {
        }
// #endif // __FIT_STEM_TRACE
        return;
      }

      feed_data_to_processor( opened_processor_iterator->second );
    } else if ( request.operation_type == socket_close ) {
      typename opened_pool_t::iterator opened_processor_iterator = opened_pool.find( request.fd );

      if ( opened_processor_iterator == opened_pool.end() ) {
        // misc::use_syslog<LOG_DEBUG>() << HERE << ":unexpected" << endl;
// #ifdef __FIT_STEM_TRACE
        try {
          std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
            ios_base::fmtflags f = _trs->flags( ios_base::showbase );
            *_trs << HERE << ":unexpected" << endl;
#ifdef STLPORT
            _trs->flags( f );
#else
            _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
          }
        }
        catch ( ... ) {
        }
// #endif // __FIT_STEM_TRACE
        return;
      }

      processor p = opened_processor_iterator->second;

      feed_data_to_processor( p );

      for ( typename at_container_type::const_iterator k = _at_disconnect.begin(); k != _at_disconnect.end(); ++k ) {
        (*k)( *p.s );
      }

      opened_pool.erase( opened_processor_iterator );

      delete p.c;
      delete p.s;
    } else {
      // misc::use_syslog<LOG_DEBUG>() << "unexpected request operation type: " << request.operation_type << endl;
// #ifdef __FIT_STEM_TRACE
      try {
        std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
          ios_base::fmtflags f = _trs->flags( ios_base::showbase );
          *_trs << HERE << "unexpected request operation type: " << request.operation_type << endl;
#ifdef STLPORT
          _trs->flags( f );
#else
          _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
        }
      }
      catch ( ... ) {
      }
// #endif // __FIT_STEM_TRACE
    }
  }
  catch ( const std::exception& e ) {
    // misc::use_syslog<LOG_DEBUG>() << HERE << ":unexpected " << e.what() << endl;
// #ifdef __FIT_STEM_TRACE
    try {
      std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
        ios_base::fmtflags f = _trs->flags( ios_base::showbase );
        *_trs << HERE << ":unexpected " << e.what() << endl;
#ifdef STLPORT
        _trs->flags( f );
#else
        _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
// #endif // __FIT_STEM_TRACE
  }
  catch (...) {
    // misc::use_syslog<LOG_DEBUG>() << HERE << ":unexpected unknown" << endl;
// #ifdef __FIT_STEM_TRACE
    try {
      std::tr2::lock_guard<std::tr2::mutex> lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & base_t::tracefault) ) {
        ios_base::fmtflags f = _trs->flags( ios_base::showbase );
        *_trs << HERE << ":unexpected unknown" << endl;
#ifdef STLPORT
        _trs->flags( f );
#else
        _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
// #endif // __FIT_STEM_TRACE
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::clear_opened_pool()
{
  try { 
    if ( !opened_pool.empty() ) {
      std::tr2::lock_guard<std::tr2::mutex> lk(std::detail::_se_lock);
      if ( std::detail::_se_stream != 0 ) {
        *std::detail::_se_stream << HERE << " worker pool not empty, remains "
                    << opened_pool.size() << std::endl;
      }

#if 0
      for( typename opened_pool_t::iterator i = opened_pool.begin();i != opened_pool.end();++i) {
        delete i->second.s;
        delete i->second.c;
      }
#endif
    }
  }
  catch ( ... ) {
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::worker()
{
  try {
    std::tr2::this_thread::signal_handler( SIGPIPE, SIG_IGN );

    for ( ; ; ) {
      request_t request; 
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( ready_lock );

        ready_cnd.wait( lk, not_empty );
        if ( ready_queue.empty() ) {
          throw finish();
        }
        request = ready_queue.front();
        ready_queue.pop();
      }

      process_request( request );
    }
  }
  catch ( const finish& ) { 
    // clear_opened_pool();
  }
  catch ( const std::exception& e ) {
    try {
      std::tr2::lock_guard<std::tr2::mutex> lk(std::detail::_se_lock);
      if ( std::detail::_se_stream != 0 ) {
        *std::detail::_se_stream << HERE << ' ' << e.what() << std::endl;
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
    try {
      std::tr2::lock_guard<std::tr2::mutex> lk(std::detail::_se_lock);
      if ( std::detail::_se_stream != 0 ) {
        *std::detail::_se_stream << HERE << " unknown exception" << std::endl;
      }
    }
    catch ( ... ) {
    }
  }
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
void connect_processor<Connect, charT, traits, _Alloc, C>::_stop()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( ready_lock );

  _in_work = false;
  ready_cnd.notify_one();
}

template <class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
std::ostream* connect_processor<Connect, charT, traits, _Alloc, C>::settrs( std::ostream* s )
{
  std::tr2::lock_guard<std::tr2::mutex> _x1( _lock_tr );
  std::ostream* tmp = _trs;
  _trs = s;

  return tmp;
}

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
std::tr2::mutex connect_processor<Connect, charT, traits, _Alloc, C>::_lock_tr;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
unsigned connect_processor<Connect, charT, traits, _Alloc, C>::_trflags = 0;

template<class Connect, class charT, class traits, class _Alloc, void (Connect::*C)( std::basic_sockstream<charT,traits,_Alloc>& )>
std::ostream* connect_processor<Connect, charT, traits, _Alloc, C>::_trs = 0;

template <class Packet, class charT, class traits, class _Alloc, void (Packet::*C)(std::basic_sockstream<charT,traits,_Alloc>&)>
typename packet_processor<Packet, charT, traits, _Alloc, C>::base_t::sockbuf_t* packet_processor<Packet, charT, traits, _Alloc, C>::operator()(sock_base::socket_type fd, const sockaddr& addr)
{
  if (!packet.is_open()) {
    packet.attach(fd, base_t::stype());
  }

  return packet.is_open() ? packet.rdbuf() : 0;
}

template <class Packet, class charT, class traits, class _Alloc, void (Packet::*C)(std::basic_sockstream<charT,traits,_Alloc>&)>
void packet_processor<Packet, charT, traits, _Alloc, C>::operator()(sock_base::socket_type fd)
{ (proc.*C)(packet); }

} // namespace std

#undef __EXTRA_SOCK_OPT
#undef __EXTRA_SOCK_OPT1

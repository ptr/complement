// -*- C++ -*- Time-stamp: <08/12/20 00:31:51 ptr>

/*
 *
 * Copyright (c) 2006, 2008
 * Petr Ovtchenkov
 *
 * Copyright (c) 2004
 * Kaspersky Lab
 *
 * Copyright (c) 1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * Licensed under the Academic Free License Version 3.0
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

#ifndef __MT_LFSTREAM_H
#define __MT_LFSTREAM_H


#include <config/feature.h>

#include <fstream>

/*
 * Note: POSIX fcntl-based file locking don't provide
 * file lock within same process.
 *
 * The BSD-style flock is free from fcntl's disadvantage,
 * but not work properly in MT-applications on FreeBSD
 * (ironic!) and depends upon file system (not work with
 * NFS in most cases, SMBFS, FAT*, may be with NTFS too).
 *
 * In case of non-MT-safe application the fcntl approach
 * has advantage (FS-independent).
 *
 */

namespace std {

namespace detail {

class __flock
{
  protected:
    void rdlock( int fd );
    void lock( int fd );
    bool try_rdlock( int fd );
    bool try_lock( int fd );
    void unlock( int fd );
};

} // namespace detail

template <class _CharT, class _Traits>
class basic_lfilebuf :
        public std::basic_filebuf<_CharT, _Traits>,
        private std::detail::__flock
{
  public:
    typedef _CharT                     char_type;
    typedef typename _Traits::int_type int_type;
    typedef typename _Traits::pos_type pos_type;
    typedef typename _Traits::off_type off_type;
    typedef _Traits                    traits_type;

  private:
    typedef std::detail::__flock flock_type;

  public:
    basic_lfilebuf() :
        basic_filebuf<_CharT,_Traits>(),
        __flock()
      { }

    ~basic_lfilebuf()
      { }

    void rdlock()
      { flock_type::rdlock( this->fd() ); }

    void lock()
      { flock_type::lock( this->fd() ); }

    void unlock()
      { flock_type::unlock( this->fd() ); }

    bool try_rdlock()
      { flock_type::try_rdlock( this->fd() ); }

    bool try_lock()
      { flock_type::try_lock( this->fd() ); }

#ifndef STLPORT
  public:
# ifdef __GNUC__ // libstd++ expected
    int fd()
      { return this->_M_file.fd(); }
# endif
# ifdef WIN32
    int fd() const
      { return this->_File->_file; }
# endif
#endif
};

template <class _CharT, class _Traits>
class basic_ilfstream :
        public std::basic_istream<_CharT, _Traits>
{
  public:
    typedef _CharT                     char_type;
    typedef typename _Traits::int_type int_type;
    typedef typename _Traits::pos_type pos_type;
    typedef typename _Traits::off_type off_type;
    typedef _Traits                    traits_type;

  public:
    basic_ilfstream() : 
        basic_ios<_CharT, _Traits>(),
        basic_istream<_CharT, _Traits>(0),
        _M_buf()
      { this->init(&_M_buf); }

    explicit basic_ilfstream( const char* __s, std::ios_base::openmode __mod = std::ios_base::in ) : 
        basic_ios<_CharT, _Traits>(),
        basic_istream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __mod | std::ios_base::in)) {
          this->setstate(std::ios_base::failbit);
        }
      }

# ifndef _STLP_NO_EXTENSIONS
    explicit basic_ilfstream( int __id, std::ios_base::openmode __mod = std::ios_base::in ) : 
        basic_ios<_CharT, _Traits>(),
        basic_istream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__id, __mod | std::ios_base::in)) {
          this->setstate(std::ios_base::failbit);
        }
      }

    basic_ilfstream( const char* __s, std::ios_base::openmode __m, long __protection ) :
        basic_ios<_CharT, _Traits>(),
        basic_istream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __m | std::ios_base::in, __protection)) {
          this->setstate(ios_base::failbit);
        }
      }
  
# endif

    ~basic_ilfstream()
      { }

  public:
    basic_lfilebuf<_CharT, _Traits>* rdbuf() const
      { return const_cast<basic_lfilebuf<_CharT, _Traits>*>(&_M_buf); }

    bool is_open()
      { return this->rdbuf()->is_open(); }

    void open(const char* __s, std::ios_base::openmode __mod = std::ios_base::in)
      {
        if (!this->rdbuf()->open(__s, __mod | std::ios_base::in)) {
          this->setstate(std::ios_base::failbit);
        }
      }

    void close()
      {
        if (!this->rdbuf()->close()) {
          this->setstate(std::ios_base::failbit);
        }
      }

    void rdlock()
      {
        if ( is_open() ) {
          this->rdbuf()->rdlock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    void lock()
      {
        if ( is_open() ) {
          this->rdbuf()->lock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    bool try_rdlock()
      {
        if ( is_open() ) {
          return this->rdbuf()->try_rdlock();
        } else {
          this->setstate(std::ios_base::failbit);
          return false;
        }
      }

    bool try_lock()
      {
        if ( is_open() ) {
          return this->rdbuf()->try_lock();
        } else {
          this->setstate(std::ios_base::failbit);
          return false;
        }
      }

    void unlock()
      {
        if ( is_open() ) {
          this->rdbuf()->unlock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    std::basic_ilfstream<_CharT, _Traits>&
    operator>>(std::basic_ilfstream<_CharT, _Traits>& (*__f)(std::basic_ilfstream<_CharT, _Traits>&) )
      { return __f(*this); }

  private:
    std::basic_lfilebuf<_CharT, _Traits> _M_buf;
};


template <class _CharT, class _Traits>
class basic_olfstream :
        public std::basic_ostream<_CharT, _Traits>
{
  public:
    typedef _CharT                     char_type;
    typedef typename _Traits::int_type int_type;
    typedef typename _Traits::pos_type pos_type;
    typedef typename _Traits::off_type off_type;
    typedef _Traits                    traits_type;

  public:
    basic_olfstream() :
        basic_ios<_CharT, _Traits>(), 
        basic_ostream<_CharT, _Traits>(0), _M_buf()
      { this->init(&_M_buf); }

    explicit basic_olfstream( const char* __s, std::ios_base::openmode __mod = std::ios_base::out) :
        basic_ios<_CharT, _Traits>(),
        basic_ostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __mod | std::ios_base::out)) {
          this->setstate(std::ios_base::failbit);
        }
      }

# ifndef _STLP_NO_EXTENSIONS
    explicit basic_olfstream(int __id, std::ios_base::openmode __mod = std::ios_base::out) :
        basic_ios<_CharT, _Traits>(),
        basic_ostream<_CharT, _Traits>(0),
        _M_buf()
      {
 	this->init(&_M_buf);
 	if (!_M_buf.open(__id, __mod | std::ios_base::out)) {
 	  this->setstate(std::ios_base::failbit);
        }
      }
    basic_olfstream(const char* __s, std::ios_base::openmode __m, long __protection) :
        basic_ios<_CharT, _Traits>(),
        basic_ostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __m | std::ios_base::out, __protection)) {
          this->setstate(std::ios_base::failbit);
        }
      }
# endif
  
    ~basic_olfstream()
      { }

  public:
    basic_lfilebuf<_CharT, _Traits>* rdbuf() const
      { return const_cast<basic_lfilebuf<_CharT, _Traits>*>(&_M_buf); } 

    bool is_open()
      { return this->rdbuf()->is_open(); }

    void open(const char* __s, std::ios_base::openmode __mod = std::ios_base::out)
      {
        if (!this->rdbuf()->open(__s, __mod | std::ios_base::out)) {
          this->setstate(std::ios_base::failbit);
        }
      }

    void rdlock()
      {
        if ( is_open() ) {
          this->rdbuf()->rdlock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    void lock()
      {
        if ( is_open() ) {
          this->rdbuf()->lock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    bool try_rdlock()
      {
        if ( is_open() ) {
          return this->rdbuf()->try_rdlock();
        } else {
          this->setstate(std::ios_base::failbit);
          return false;
        }
      }

    bool try_lock()
      {
        if ( is_open() ) {
          return this->rdbuf()->try_lock();
        } else {
          this->setstate(std::ios_base::failbit);
          return false;
        }
      }

    void unlock()
      {
        if ( is_open() ) {
          this->rdbuf()->unlock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    std::basic_olfstream<_CharT, _Traits>&
    operator<<(std::basic_olfstream<_CharT, _Traits>& (*__f)(std::basic_olfstream<_CharT, _Traits>&) )
      { return __f(*this); }

  private:
    std::basic_lfilebuf<_CharT, _Traits> _M_buf;
};


template <class _CharT, class _Traits>
class basic_lfstream :
        public std::basic_iostream<_CharT, _Traits>
{
  public:
    typedef _CharT                     char_type;
    typedef typename _Traits::int_type int_type;
    typedef typename _Traits::pos_type pos_type;
    typedef typename _Traits::off_type off_type;
    typedef _Traits                    traits_type;

  public:
  
    basic_lfstream() :
        basic_ios<_CharT, _Traits>(),
        basic_iostream<_CharT, _Traits>(0),
        _M_buf()
      { this->init(&_M_buf); }

    explicit basic_lfstream(const char* __s, std::ios_base::openmode __mod = std::ios_base::in | std::ios_base::out) :
        basic_ios<_CharT, _Traits>(),
        basic_iostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __mod)) {
          this->setstate(std::ios_base::failbit);
        }
      }

# ifndef _STLP_NO_EXTENSIONS
    explicit basic_lfstream(int __id, std::ios_base::openmode __mod = std::ios_base::in | std::ios_base::out) :
        basic_ios<_CharT, _Traits>(),
        basic_iostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__id, __mod)) {
          this->setstate(ios_base::failbit);
        }
      }
    basic_lfstream(const char* __s, std::ios_base::openmode __m, long __protection) :
        basic_ios<_CharT, _Traits>(),
        basic_iostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __m, __protection)) {
          this->setstate(ios_base::failbit);  
        }
      }
# endif    
    ~basic_lfstream() {}

  public:
    basic_lfilebuf<_CharT, _Traits>* rdbuf() const
      { return const_cast<basic_lfilebuf<_CharT, _Traits>*>(&_M_buf); } 

    bool is_open()
      { return this->rdbuf()->is_open(); }

    void open(const char* __s, std::ios_base::openmode __mod = std::ios_base::in | std::ios_base::out)
      {
        if (!this->rdbuf()->open(__s, __mod)) {
          this->setstate(ios_base::failbit);
        }
      }

    void close()
      {
        if (!this->rdbuf()->close()) {
          this->setstate(std::ios_base::failbit);
        }
      }

    void rdlock()
      {
        if ( is_open() ) {
          this->rdbuf()->rdlock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    void lock()
      {
        if ( is_open() ) {
          this->rdbuf()->lock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    bool try_rdlock()
      {
        if ( is_open() ) {
          return this->rdbuf()->try_rdlock();
        } else {
          this->setstate(std::ios_base::failbit);
          return false;
        }
      }

    bool try_lock()
      {
        if ( this->is_open() ) {
          return this->rdbuf()->try_lock();
        } else {
          this->setstate(std::ios_base::failbit);
          return false;
        }
      }

    void unlock()
      {
        if ( is_open() ) {
          this->rdbuf()->unlock();
        } else {
          this->setstate(std::ios_base::failbit);
        }
      }

    std::basic_lfstream<_CharT, _Traits>&
    operator<<(std::basic_lfstream<_CharT, _Traits>& (*__f)(std::basic_lfstream<_CharT, _Traits>&) )
      { return __f(*this); }

    std::basic_lfstream<_CharT, _Traits>&
    operator>>(std::basic_lfstream<_CharT, _Traits>& (*__f)(std::basic_lfstream<_CharT, _Traits>&) )
      { return __f(*this); }

  private:
    /* mutable */ std::basic_lfilebuf<_CharT, _Traits> _M_buf;
};

typedef basic_ilfstream<char,char_traits<char> > ilfstream;
typedef basic_olfstream<char,char_traits<char> > olfstream;
typedef basic_lfstream<char,char_traits<char> >  lfstream;

template <class _CharT, class _Traits>
inline basic_ilfstream<_CharT,_Traits>& lock( basic_ilfstream<_CharT,_Traits>& _s )
{ _s.lock(); return _s; }

template <class _CharT, class _Traits>
inline basic_ilfstream<_CharT,_Traits>& rdlock( basic_ilfstream<_CharT,_Traits>& _s )
{ _s.rdlock(); return _s; }

template <class _CharT, class _Traits>
inline basic_ilfstream<_CharT,_Traits>& unlock( basic_ilfstream<_CharT,_Traits>& _s )
{ _s.unlock(); return _s; }

template <class _CharT, class _Traits>
inline basic_olfstream<_CharT,_Traits>& lock( basic_olfstream<_CharT,_Traits>& _s )
{ _s.lock(); return _s; }

template <class _CharT, class _Traits>
inline basic_olfstream<_CharT,_Traits>& rdlock( basic_olfstream<_CharT,_Traits>& _s )
{ _s.rdlock(); return _s; }

template <class _CharT, class _Traits>
inline basic_olfstream<_CharT,_Traits>& unlock( basic_olfstream<_CharT,_Traits>& _s )
{ _s.unlock(); return _s; }

template <class _CharT, class _Traits>
inline basic_lfstream<_CharT,_Traits>& lock( basic_lfstream<_CharT,_Traits>& _s )
{ _s.lock(); return _s; }

template <class _CharT, class _Traits>
inline basic_lfstream<_CharT,_Traits>& rdlock( basic_lfstream<_CharT,_Traits>& _s )
{ _s.rdlock(); return _s; }

template <class _CharT, class _Traits>
inline basic_lfstream<_CharT,_Traits>& unlock( basic_lfstream<_CharT,_Traits>& _s )
{ _s.unlock(); return _s; }

} // namespace std

#endif // __MT_LFSTREAM_H

// -*- C++ -*- Time-stamp: <04/04/23 18:03:27 ptr>

/*
 *
 * Copyright (c) 2004
 * Kaspersky Labs
 *
 * Copyright (c) 1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * Licensed under the Academic Free License Version 2.0
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

#ifndef __mt_lfstream_h
#define __mt_lfstream_h

#include <mt/flck.h>
#include <fstream>

#if defined(WIN32) && !defined(_STLPORT_VERSION) && !defined(__GNUC__)
# include <mt/_fbuf.h>
#endif

#ifndef _STLPORT_VERSION
# define _STLP_BEGIN_NAMESPACE namespace std {
# define _STLP_END_NAMESPACE }
# define __CONST_CAST(t,v) const_cast<t>(v)
#endif

_STLP_BEGIN_NAMESPACE

template <class _CharT, class _Traits>
class basic_lfilebuf :
#if defined(WIN32) && !defined(_STLPORT_VERSION) && !defined(__GNUC__)
    public __basic_filebuf<_CharT, _Traits>
#else
    public basic_filebuf<_CharT, _Traits>
#endif
{
  public:                         // Types.
    typedef _CharT                     char_type;
    typedef typename _Traits::int_type int_type;
    typedef typename _Traits::pos_type pos_type;
    typedef typename _Traits::off_type off_type;
    typedef _Traits                    traits_type;

    typedef typename _Traits::state_type    _State_type;
    typedef basic_filebuf<_CharT, _Traits>  _Base;
    typedef basic_lfilebuf<_CharT, _Traits> _Self;

  public:                         // Constructors, destructor.
    basic_lfilebuf() :
      basic_filebuf<_CharT,_Traits>(),
      _is_locked( false )
      { }
    ~basic_lfilebuf()
      { if ( _is_locked ) unlock(); }

    _Self *lock_ex()
      {
        if ( flck( this->fd(), _F_LCK_W ) == 0 ) {
          _is_locked = true;
          return this;
        }
        return 0;
      }

    _Self *lock_sh()
      {
        if ( flck( this->fd(), _F_LCK_R ) == 0 ) {
          _is_locked = true;
          return this;
        }
        return 0;
      }

    _Self *unlock()
      {
        if ( flck( this->fd(), _F_UNLCK ) == 0 ) {
          _is_locked = false;
          return this;
        }
        return 0;
      }

    bool is_locked() const
      { return _is_locked; }


  private:
    bool _is_locked;

#ifndef _STLPORT_VERSION
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

//----------------------------------------------------------------------
// Class basic_ifstream<>

template <class _CharT, class _Traits>
class basic_ilfstream : public basic_istream<_CharT, _Traits>
{
  public:                         // Types
    typedef _CharT                     char_type;
    typedef typename _Traits::int_type int_type;
    typedef typename _Traits::pos_type pos_type;
    typedef typename _Traits::off_type off_type;
    typedef _Traits                    traits_type;

    typedef basic_ios<_CharT, _Traits>     _Basic_ios;
    typedef basic_istream<_CharT, _Traits> _Base;
    typedef basic_lfilebuf<_CharT, _Traits> _Buf;

  public:                         // Constructors, destructor.

    basic_ilfstream() : 
        basic_ios<_CharT, _Traits>(),
        basic_istream<_CharT, _Traits>(0),
        _M_buf()
      { this->init(&_M_buf); }

    explicit basic_ilfstream(const char* __s, ios_base::openmode __mod = ios_base::in) : 
        basic_ios<_CharT, _Traits>(),
        basic_istream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __mod | ios_base::in))
          this->setstate(ios_base::failbit);
      }

# ifndef _STLP_NO_EXTENSIONS
    explicit basic_ilfstream(int __id, ios_base::openmode __mod = ios_base::in) : 
        basic_ios<_CharT, _Traits>(),
        basic_istream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__id, __mod | ios_base::in))
          this->setstate(ios_base::failbit);
      }
    basic_ilfstream(const char* __s, ios_base::openmode __m, long __protection) :
        basic_ios<_CharT, _Traits>(),
        basic_istream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __m | ios_base::in, __protection))
          this->setstate(ios_base::failbit);  
      }
  
# endif

    ~basic_ilfstream() {}

  public:                         // File and buffer operations.
    basic_lfilebuf<_CharT, _Traits>* rdbuf() const
      { return __CONST_CAST(_Buf*,&_M_buf); }

    bool is_open() {
      return this->rdbuf()->is_open();
    }

    void open(const char* __s, ios_base::openmode __mod = ios_base::in) {
      if (!this->rdbuf()->open(__s, __mod | ios_base::in))
        this->setstate(ios_base::failbit);
    }

    void close() {
      if (!this->rdbuf()->close())
        this->setstate(ios_base::failbit);
    }

    void lock_sh()
      {
        if ( !is_open() || !this->rdbuf()->lock_sh() ) {
          this->setstate(ios_base::failbit);
        }
      }

    void unlock()
      {
        if ( !is_open() || !this->rdbuf()->is_locked() || !this->rdbuf()->unlock() ) {
          this->setstate(ios_base::failbit);
        }
      }

    basic_ilfstream<_CharT, _Traits>&
    operator>>(basic_ilfstream<_CharT, _Traits>& (*__f)(basic_ilfstream<_CharT, _Traits>&) )
      { return __f(*this); }

  private:
    basic_lfilebuf<_CharT, _Traits> _M_buf;
};


//----------------------------------------------------------------------
// Class basic_ofstream<>

template <class _CharT, class _Traits>
class basic_olfstream : public basic_ostream<_CharT, _Traits>
{
  public:                         // Types
    typedef _CharT                     char_type;
    typedef typename _Traits::int_type int_type;
    typedef typename _Traits::pos_type pos_type;
    typedef typename _Traits::off_type off_type;
    typedef _Traits                    traits_type;

    typedef basic_ios<_CharT, _Traits>      _Basic_ios;
    typedef basic_ostream<_CharT, _Traits>  _Base;
    typedef basic_lfilebuf<_CharT, _Traits> _Buf;

  public:                         // Constructors, destructor.
    basic_olfstream() :
        basic_ios<_CharT, _Traits>(), 
        basic_ostream<_CharT, _Traits>(0), _M_buf()
      { this->init(&_M_buf); }
    explicit basic_olfstream(const char* __s, ios_base::openmode __mod = ios_base::out) :
        basic_ios<_CharT, _Traits>(),
        basic_ostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __mod | ios_base::out))
          this->setstate(ios_base::failbit);
      }

# ifndef _STLP_NO_EXTENSIONS
    explicit basic_olfstream(int __id, ios_base::openmode __mod = ios_base::out) :
        basic_ios<_CharT, _Traits>(),
        basic_ostream<_CharT, _Traits>(0),
        _M_buf()
      {
 	this->init(&_M_buf);
 	if (!_M_buf.open(__id, __mod | ios_base::out))
 	  this->setstate(ios_base::failbit);
      }
    basic_olfstream(const char* __s, ios_base::openmode __m, long __protection) :
        basic_ios<_CharT, _Traits>(),
        basic_ostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __m | ios_base::out, __protection))
          this->setstate(ios_base::failbit);  
      }
# endif
  
    ~basic_olfstream() {}

  public:                         // File and buffer operations.
    basic_lfilebuf<_CharT, _Traits>* rdbuf() const
      { return __CONST_CAST(_Buf*,&_M_buf); } 

    bool is_open()
      { return this->rdbuf()->is_open(); }

    void open(const char* __s, ios_base::openmode __mod= ios_base::out)
      {
        if (!this->rdbuf()->open(__s, __mod | ios_base::out))
          this->setstate(ios_base::failbit);
      }

    void close()
      {
        if (!this->rdbuf()->close())
          this->setstate(ios_base::failbit);
      }

    void lock_ex()
      {
        if ( !is_open() || !this->rdbuf()->lock_ex() ) {
          this->setstate(ios_base::failbit);
        }
      }

    void unlock()
      {
        if ( !is_open() || !this->rdbuf()->is_locked() || !this->rdbuf()->unlock() ) {
          this->setstate(ios_base::failbit);
        }
      }

    basic_olfstream<_CharT, _Traits>&
    operator<<(basic_olfstream<_CharT, _Traits>& (*__f)(basic_olfstream<_CharT, _Traits>&) )
      { return __f(*this); }

  private:
    basic_lfilebuf<_CharT, _Traits> _M_buf;
};


//----------------------------------------------------------------------
// Class basic_fstream<>

template <class _CharT, class _Traits>
class basic_lfstream : public basic_iostream<_CharT, _Traits>
{
  public:                         // Types
    typedef _CharT                     char_type;
    typedef typename _Traits::int_type int_type;
    typedef typename _Traits::pos_type pos_type;
    typedef typename _Traits::off_type off_type;
    typedef _Traits                    traits_type;

    typedef basic_ios<_CharT, _Traits>       _Basic_ios;
    typedef basic_iostream<_CharT, _Traits>  _Base;
    typedef basic_filebuf<_CharT, _Traits>   _Buf;

  public:                         // Constructors, destructor.
  
    basic_lfstream() :
        basic_ios<_CharT, _Traits>(),
        basic_iostream<_CharT, _Traits>(0),
        _M_buf()
      { this->init(&_M_buf); }

    explicit basic_lfstream(const char* __s, ios_base::openmode __mod = ios_base::in | ios_base::out) :
        basic_ios<_CharT, _Traits>(),
        basic_iostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __mod))
          this->setstate(ios_base::failbit);
      }

# ifndef _STLP_NO_EXTENSIONS
    explicit basic_lfstream(int __id, ios_base::openmode __mod = ios_base::in | ios_base::out) :
        basic_ios<_CharT, _Traits>(),
        basic_iostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__id, __mod))
          this->setstate(ios_base::failbit);
      }
    basic_lfstream(const char* __s, ios_base::openmode __m, long __protection) :
        basic_ios<_CharT, _Traits>(),
        basic_iostream<_CharT, _Traits>(0),
        _M_buf()
      {
        this->init(&_M_buf);
        if (!_M_buf.open(__s, __m, __protection))
          this->setstate(ios_base::failbit);  
      }
# endif    
    ~basic_lfstream() {}

  public:                         // File and buffer operations.

    basic_lfilebuf<_CharT, _Traits>* rdbuf() const
      { return __CONST_CAST(_Buf*,&_M_buf); } 

    bool is_open()
      { return this->rdbuf()->is_open(); }

    void open(const char* __s, ios_base::openmode __mod = ios_base::in | ios_base::out)
      {
        if (!this->rdbuf()->open(__s, __mod))
          this->setstate(ios_base::failbit);
      }

    void close()
      {
        if (!this->rdbuf()->close())
          this->setstate(ios_base::failbit);
      }

    void lock_ex()
      {
        if ( !is_open() || !this->rdbuf()->lock_ex() ) {
          this->setstate(ios_base::failbit);
        }
      }

    void lock_sh()
      {
        if ( !is_open() || !this->rdbuf()->lock_sh() ) {
          this->setstate(ios_base::failbit);
        }
      }

    void unlock()
      {
        if ( !is_open() || !this->rdbuf()->is_locked() || !this->rdbuf()->unlock() ) {
          this->setstate(ios_base::failbit);
        }
      }

    basic_lfstream<_CharT, _Traits>&
    operator<<(basic_lfstream<_CharT, _Traits>& (*__f)(basic_lfstream<_CharT, _Traits>&) )
      { return __f(*this); }

    basic_lfstream<_CharT, _Traits>&
    operator>>(basic_lfstream<_CharT, _Traits>& (*__f)(basic_lfstream<_CharT, _Traits>&) )
      { return __f(*this); }

  private:
    basic_lfilebuf<_CharT, _Traits> _M_buf;
};

typedef basic_ilfstream<char,char_traits<char> > ilfstream;
typedef basic_olfstream<char,char_traits<char> > olfstream;
typedef basic_lfstream<char,char_traits<char> >  lfstream;


template <class _CharT, class _Traits>
inline basic_ilfstream<_CharT,_Traits>& slck( basic_ilfstream<_CharT,_Traits>& _s )
{ _s.lock_sh(); return _s; }

template <class _CharT, class _Traits>
inline basic_ilfstream<_CharT,_Traits>& ulck( basic_ilfstream<_CharT,_Traits>& _s )
{ _s.unlock(); return _s; }

template <class _CharT, class _Traits>
inline basic_olfstream<_CharT,_Traits>& elck( basic_olfstream<_CharT,_Traits>& _s )
{ _s.lock_ex(); return _s; }

template <class _CharT, class _Traits>
inline basic_olfstream<_CharT,_Traits>& ulck( basic_olfstream<_CharT,_Traits>& _s )
{ _s.unlock(); return _s; }

template <class _CharT, class _Traits>
inline basic_lfstream<_CharT,_Traits>& slck( basic_lfstream<_CharT,_Traits>& _s )
{ _s.lock_sh(); return _s; }

template <class _CharT, class _Traits>
inline basic_lfstream<_CharT,_Traits>& elck( basic_lfstream<_CharT,_Traits>& _s )
{ _s.lock_ex(); return _s; }

template <class _CharT, class _Traits>
inline basic_lfstream<_CharT,_Traits>& ulck( basic_lfstream<_CharT,_Traits>& _s )
{ _s.unlock(); return _s; }


_STLP_END_NAMESPACE

#endif // __mt_lfstream_h

// -*- C++ -*- Time-stamp: <06/12/26 10:24:35 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __mt_shm_h
#define __mt_shm_h

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <stdexcept>

#include <stl/type_traits.h>

#include <mt/xmt.h>

namespace xmt {

struct shm_base
{
  enum open_type {
    create = IPC_CREAT,
    exclusive = IPC_EXCL
  };

  enum flags_type {
    round = SHM_RND,
    ro    = SHM_RDONLY,
  };
};

class shm_bad_alloc :
        public std::bad_alloc
{
  public:
    shm_bad_alloc() throw() :
        err( errno )
      {}
    shm_bad_alloc( int s ) throw() :
        err( s )
      {}
    shm_bad_alloc( const shm_bad_alloc& a ) throw() :
        err( a.err )
      {}
    shm_bad_alloc& operator =( const shm_bad_alloc& r ) throw()
      { err = r.err; return *this; }
    virtual ~shm_bad_alloc() throw()
      {}
    virtual const char *what() const throw();

  private:
    int err;
};

template <class T>
struct ipc_sharable
{
  typedef typename std::__type_traits<T>::is_POD_type is_ipc_sharable;
};

template <int _Inst> class shm_alloc;

namespace detail {

template <int Inst>
class __shm_alloc :
        public xmt::shm_base
{
  public:
    typedef void * pointer;
    typedef const void * const_pointer;
    typedef size_t size_type;
    typedef size_t difference_type;

    __shm_alloc() throw()
      { }

    __shm_alloc( const char *name, size_type sz, int o, int mode, int f = 0, void *addr = 0 )
      { allocate( name, sz, o, mode, f, addr ); }

    __shm_alloc( key_t k, size_type sz, int o, int mode, int f = 0, void *addr = 0 )
      { allocate( k, sz, o, mode, f, addr ); }

    __shm_alloc( int id, int f = 0, void *addr = 0 )
      { allocate( id, f, addr ); }

    ~__shm_alloc() throw()
      {
        if ( _seg != reinterpret_cast<void *>(-1) ) {
          shmdt( _seg );
          _seg = reinterpret_cast<void *>(-1);
        }
      }

    static pointer allocate( const char *name, size_type sz, int o, int mode, int f = 0, void *addr = 0 );
    static pointer allocate( key_t k, size_type sz, int o, int mode, int f = 0, void *addr = 0 );
    static pointer allocate( int id, int f = 0, void *addr = 0 );

    static pointer reallocate( int f = 0, void *addr = 0 );

    static void deallocate( bool force = false )
      {
        if ( _seg != reinterpret_cast<void *>(-1) ) {
          shmdt( _seg );
          _seg = reinterpret_cast<void *>(-1);
        }
        if ( _id != -1 ) {
          if ( force || (shmctl( _id, IPC_STAT, &_ds ) == 0 && _ds.shm_nattch == 0) ) {
            shmctl( _id, IPC_RMID, &_ds );
            _id = -1;
          }
        }
      }

    static pointer address() throw()
      { return _seg; }

    static size_type max_size() throw()
      { return _id == -1 ? 0 : _ds.shm_segsz; }

  private:
    static shmid_ds _ds;
    static int _id;
    static void *_seg;
};

template <int Inst>
shmid_ds __shm_alloc<Inst>::_ds;

template <int Inst>
int __shm_alloc<Inst>::_id = -1;

template <int Inst>
void *__shm_alloc<Inst>::_seg = reinterpret_cast<void *>(-1);

template <int Inst>
typename __shm_alloc<Inst>::pointer __shm_alloc<Inst>::allocate( const char *name, size_type sz, int o, int mode, int f, void *addr )
{
  if ( _id != -1 ) {
    throw shm_bad_alloc( -1 );
  }
  int of = 0;
  if ( o & create ) {
    of |= O_CREAT;
  }
  if ( o & exclusive ) {
    of |= O_EXCL;
  }
  if ( f & SHM_RDONLY ) {
    of |= O_RDONLY;
  } else {
    of |= O_RDWR;
  }
  bool rmfile = true;
  if ( (o & create) && !(o & exclusive) ) {
    int exist_fd = open( name, of & ~O_CREAT, (0777 & mode) );
    if ( exist_fd >= 0 ) {
      rmfile = false;
      ::close( exist_fd );
    }
  }
  int fd = open( name, of, (0777 & mode) );
  if ( fd < 0 ) {
    throw shm_bad_alloc();
  }
  close( fd );
  key_t k = ftok( name, Inst );
  if ( k == -1 ) {
    if ( (o & create) && (rmfile || (o & exclusive)) ) {
      unlink( name );
    }
    throw shm_bad_alloc();
  }
  return allocate( k, sz, o, mode, f, addr );
}

template <int Inst>
typename __shm_alloc<Inst>::pointer __shm_alloc<Inst>::allocate( key_t k, size_type sz, int o, int mode, int f, void *addr )
{
  if ( _id != -1 ) {
    throw shm_bad_alloc( -1 );
  }

  static const size_type psz = getpagesize();

  _id = shmget( k, ((sz + sizeof(typename shm_alloc<Inst>::_master) + sizeof(typename shm_alloc<Inst>::_aheader) )/ psz + 1) * psz, o | (0777 & mode) );
  if ( _id == -1 ) {
    throw shm_bad_alloc();
  }
  if ( shmctl( _id, IPC_STAT, &_ds ) == -1 ) {
    throw shm_bad_alloc();
  }
  _seg = shmat( _id, addr, f );
  if ( _seg == reinterpret_cast<void *>(-1) ) {
    if ( (o & create) && (o & exclusive) ) {
      shmctl( _id, IPC_RMID, &_ds );
    }
    throw shm_bad_alloc();
  }

  return _seg;
}

template <int Inst>
typename __shm_alloc<Inst>::pointer __shm_alloc<Inst>::allocate( int id, int f, void *addr )
{
  if ( _id != -1 ) {
    throw shm_bad_alloc( -1 );
  }
  if ( shmctl( id, IPC_STAT, &_ds ) == -1 ) {
    throw shm_bad_alloc();
  }
  _id = id;
  _seg = shmat( _id, addr, f );
  if ( _seg == reinterpret_cast<void *>(-1) ) {
    throw shm_bad_alloc();
  }

  return _seg;
}

template <int Inst>
typename __shm_alloc<Inst>::pointer __shm_alloc<Inst>::reallocate( int f, void *addr )
{
  if ( _id == -1 ) {
    throw shm_bad_alloc( -1 );
  }
  if ( shmctl( _id, IPC_STAT, &_ds ) == -1 ) {
    throw shm_bad_alloc();
  }
  _seg = shmat( _id, addr, f );
  if ( _seg == reinterpret_cast<void *>(-1) ) {
    throw shm_bad_alloc();
  }

  return _seg;
}

} // detail

template <int _Inst>
class shm_alloc
{
  protected:
    typedef void * pointer;
    typedef const void * const_pointer;
    typedef typename detail::__shm_alloc<_Inst>::size_type size_type;

    struct _master
    {
        uint64_t _magic;
        size_type _first;
        xmt::__Mutex<false,true> _lock;
    };

    struct _fheader
    {
        size_type _sz;
        size_type _next;
    };

    struct _aheader
    {
        size_type _sz;
    };

    enum {
      __align = sizeof(int)
    };


  public:
    static void allocate( const char *name, size_type sz, int o, int mode, int f = 0, void *addr = 0 )
      {
        _master *m = reinterpret_cast<_master *>( _seg.allocate( name, sz, o, mode, f, addr ) );
        if ( m->_magic != MAGIC ) {
          init( *m );
        }
      }

    static void allocate( key_t k, size_type sz, int o, int mode, int f = 0, void *addr = 0 )
      {
        _master *m = reinterpret_cast<_master *>( _seg.allocate( k, sz, o, mode, f, addr ) );
        if ( m->_magic != MAGIC ) {
          init( *m );
        }
      }

    static void deallocate( bool force = false )
      { _seg.deallocate( force ); }

    size_type max_size() const throw()
      { return _seg.max_size() == 0 ? 0 : (_seg.max_size() - sizeof(_master) - sizeof(_aheader)); }

  protected:
    static void *allocate( size_type n, void *hint = 0 )
      {
        _master *m = reinterpret_cast<_master *>( _seg.address() );
        if ( m != reinterpret_cast<_master *>(-1) ) {
          xmt::__Locker<xmt::__Mutex<false,true> > lk( m->_lock );
          return _traverse( &m->_first, n );
        }

        throw shm_bad_alloc( -2 );

        return 0;
      }

    static void deallocate( pointer p, size_type n );
    static void *_traverse( size_type *_prev, size_type n );

  private:
    static void init( _master& m )
      {
        m._magic = MAGIC;
        new ( &m._lock ) xmt::__Mutex<false,true>();
        xmt::__Locker<xmt::__Mutex<false,true> > lk( m._lock );
        m._first = sizeof( _master );
        _fheader& h = *new ( reinterpret_cast<char *>(&m) + sizeof(_master) ) _fheader();
        h._next = 0;
        h._sz = _seg.max_size() - sizeof( _master ) - sizeof( _aheader );
      }

  private:
    static const uint64_t MAGIC;
    static detail::__shm_alloc<_Inst> _seg;

    friend class detail::__shm_alloc<_Inst>;
};

template <int _Inst>
void shm_alloc<_Inst>::deallocate( pointer p, size_type n )
{
  n += (__align - n % __align) % __align;
  _master *m = reinterpret_cast<_master *>( _seg.address() );
  if ( m != reinterpret_cast<_master *>(-1) && (reinterpret_cast<char *>(p) - reinterpret_cast<char *>(_seg.address())) < (_seg.max_size() + sizeof(_master) + sizeof(_aheader) ) ) {
    xmt::__Locker<xmt::__Mutex<false,true> > lk( m->_lock );
    _aheader *a = reinterpret_cast<_aheader *>( reinterpret_cast<char *>(p) - sizeof(_aheader) );
    size_type off = reinterpret_cast<char *>(p) - reinterpret_cast<char *>(_seg.address());
    if ( m->_first == 0 ) {
      m->_first = off - sizeof(_aheader);
      reinterpret_cast<_fheader *>(a)->_next = 0;
    } else {
      _fheader *h;
      for ( h = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + m->_first); h->_next != 0;
            h = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + h->_next) ) {
        if ( h->_next < off ) { // the case h->_next == off is illegal, due to shift on sizeof(_aheader)
          continue;
        }
        if ( reinterpret_cast<char *>(&h->_next) + h->_sz + sizeof(_aheader) == p ) { // attach to prev block
          if ( h->_next == (off + a->_sz) ) { // glue with following free block
            _fheader *nextheader = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + h->_next);
            h->_sz += a->_sz + sizeof(_aheader) * 2  + nextheader->_sz;
            h->_next = nextheader->_next;
          } else {
            h->_sz += a->_sz + sizeof(_aheader);
          }
        } else if ( h->_next == (off + a->_sz) ) { // glue with following free block
          _fheader *nextheader = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + h->_next);
          reinterpret_cast<_fheader *>(a)->_next = nextheader->_next;
          reinterpret_cast<_fheader *>(a)->_sz += sizeof(_aheader) + nextheader->_sz;
        } else {
          reinterpret_cast<_fheader *>(a)->_next = h->_next;
          h->_next = off - sizeof(_aheader);
        }
        return;
      }
      // before first free block or after last free block
      // Note, that list of free blocks not empty here!
      if ( off > (reinterpret_cast<char *>(h) - reinterpret_cast<char *>(_seg.address())) ) { // become last free block
        if ( reinterpret_cast<pointer>(reinterpret_cast<char *>(h) + sizeof(_aheader) * 2 + h->_sz) == p ) {
          // glue with last free block
          h->_sz += a->_sz + sizeof(_aheader);
        } else {
          h->_next = off - sizeof(_aheader);
          reinterpret_cast<_fheader *>(a)->_next = 0;
        }
      } else { // become first free block
        if ( m->_first == off + a->_sz ) { // glue with next free block
          _fheader *nextheader = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + m->_first);
          reinterpret_cast<_fheader *>(a)->_next = nextheader->_next;
          reinterpret_cast<_fheader *>(a)->_sz += sizeof(_aheader) + nextheader->_sz;
        } else { // link to next free block
          reinterpret_cast<_fheader *>(a)->_next = m->_first;
        }
        m->_first = off - sizeof(_aheader);
      }
    }
  }
}

template <int _Inst>
void *shm_alloc<_Inst>::_traverse( size_type *_prev, size_type n )
{
  n += (__align - n % __align) % __align;
  for ( _fheader *h = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + *_prev); *_prev != 0;
        _prev = &h->_next, h = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + *_prev)) {
    if ( h->_sz > (n + sizeof( _fheader )) ) { // reduce this free block, write new free header
      *_prev += n + sizeof( _aheader );
      _fheader *hnew = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + *_prev );
      hnew->_sz = h->_sz - n - sizeof( _aheader );
      hnew->_next = h->_next;
      reinterpret_cast<_aheader *>(h)->_sz = n;
      return reinterpret_cast<void *>(reinterpret_cast<char *>(h) + sizeof( _aheader ));
    } else if ( h->_sz >= n ) { // this block is too small to split it; use it in whole
      *_prev = h->_next;
      reinterpret_cast<_aheader *>(h)->_sz = h->_sz;
      return reinterpret_cast<void *>(reinterpret_cast<char *>(h) + sizeof( _aheader ));
    }
  }

  throw shm_bad_alloc( -3 );

  return 0;
}


template <int _Inst>
const uint64_t shm_alloc<_Inst>::MAGIC = 0xaa99665500000000ULL + _Inst;

template <int _Inst>
detail::__shm_alloc<_Inst> shm_alloc<_Inst>::_seg;

template <class _Tp, int _Inst>
class allocator_shm :
    public shm_alloc<_Inst>
{
  public:
    typedef shm_alloc<_Inst> chunk_type;
    typedef _Tp        value_type;
    typedef _Tp*       pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp&       reference;
    typedef const _Tp& const_reference;
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;

    template <class _Tp1, int _Inst1>
    struct rebind
    {
        typedef allocator_shm<_Tp1, _Inst1> other;
    };

    allocator_shm() throw()
      {}

    template <class _Tp1, int _Inst1>
    allocator_shm(const allocator_shm<_Tp1, _Inst1>&) throw()
      {}

    allocator_shm(const allocator_shm<_Tp,_Inst>&) throw()
      {}

    // allocator_shm(__move_source<allocator_shm<_Tp> > src) throw()
    //  {}

    ~allocator_shm() throw()
      {}

    pointer address(reference __x) const
      {return &__x;}

    const_pointer address(const_reference __x) const
      { return &__x; }

    // __n is permitted to be 0.  The C++ standard says nothing about what the return value is when __n == 0.
    _Tp* allocate( size_type __n, const void* = 0 )
      {
        if ( __n > max_size() ) {
          throw shm_bad_alloc();
        }
        if ( __n != 0 ) {
          return reinterpret_cast<_Tp *>( chunk_type::allocate( __n * sizeof(value_type) ) );
        }
        return 0;
      }
    // __p is permitted to be a null pointer, only if n==0.
    void deallocate( pointer __p, size_type __n )
      {
        if ( __p != 0 ) {
          chunk_type::deallocate( __p, __n * sizeof(value_type) );
        }
      }
    // backwards compatibility
    void deallocate( pointer __p ) const
      {
        if ( __p != 0 ) {
          chunk_type::deallocate( __p, sizeof(value_type) );
        }
      }

    size_type max_size() const throw()
      { return chunk_type::max_size() / sizeof(value_type); }

    void construct(pointer __p, const_reference __val)
      { _STLP_STD::_Copy_Construct(__p, __val); }

    void destroy(pointer __p)
      { _STLP_STD::_Destroy(__p); }

};

} // namespace xmt

#endif // __mt_shm_h

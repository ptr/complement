// -*- C++ -*- Time-stamp: <07/08/03 09:47:53 ptr>

/*
 * Copyright (c) 2006, 2007
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
#include <algorithm>

#include <misc/type_traits.h>

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
struct is_ipc_sharable :
    public std::tr1::false_type
{ };

#define  __SPEC_(C,T,B)               \
template <>                           \
struct C<T > :                        \
    public std::tr1::integral_constant<bool, B> \
{ }

#define __SPEC_FULL(C,T,B) \
__SPEC_(C,T,B);            \
__SPEC_(C,volatile T,B);

__SPEC_FULL(is_ipc_sharable,bool,true);
__SPEC_FULL(is_ipc_sharable,char,true);
__SPEC_FULL(is_ipc_sharable,signed char,true);
__SPEC_FULL(is_ipc_sharable,unsigned char,true);
__SPEC_FULL(is_ipc_sharable,wchar_t,true);
__SPEC_FULL(is_ipc_sharable,short,true);
__SPEC_FULL(is_ipc_sharable,unsigned short,true);
__SPEC_FULL(is_ipc_sharable,int,true);
__SPEC_FULL(is_ipc_sharable,unsigned int,true);
__SPEC_FULL(is_ipc_sharable,long,true);
__SPEC_FULL(is_ipc_sharable,unsigned long,true);
__SPEC_FULL(is_ipc_sharable,long long,true);
__SPEC_FULL(is_ipc_sharable,unsigned long long,true);
__SPEC_FULL(is_ipc_sharable,float,true);
__SPEC_FULL(is_ipc_sharable,double,true);
__SPEC_FULL(is_ipc_sharable,long double,true);
__SPEC_FULL(is_ipc_sharable,xmt::__condition<true>,true);
__SPEC_FULL(is_ipc_sharable,xmt::__semaphore<true>,true);
__SPEC_FULL(is_ipc_sharable,xmt::__barrier<true>,true);
__SPEC_FULL(is_ipc_sharable,xmt::shared_mutex,true);
__SPEC_FULL(is_ipc_sharable,xmt::shared_recursive_mutex,true);

#undef __SPEC_FULL
#undef __SPEC_

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

    static size_type count() throw()
      {
        if ( _id != -1 ) {
          if ( shmctl( _id, IPC_STAT, &_ds ) == 0 ) {
            return _ds.shm_nattch;
          }
        }
        return 0;
      }

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

template <int _Inst> class shm_alloc;

template <int _Inst>
class shm_name_mgr
{
  public:
   typedef typename detail::__shm_alloc<_Inst>::size_type size_type;

  public:
    bool is_name_exist( int );

    template <class T>
    void named( const T& obj, int name )
    {
      xmt::basic_lock<__mutex<false,true> > lk( _lock );
      if ( _last == 255 ) {
        throw std::range_error( "too many named objects" );
      }
      if ( (reinterpret_cast<const void *>(&obj) <= shm_alloc<_Inst>::_seg.address()) ||
           (reinterpret_cast<const void *>(&obj) > (reinterpret_cast<char *>(shm_alloc<_Inst>::_seg.address()) +
                                              shm_alloc<_Inst>::max_size() +
                                              sizeof(typename shm_alloc<_Inst>::_master) +
                                              sizeof(typename shm_alloc<_Inst>::_aheader) )) ) {
        throw std::invalid_argument( std::string("object beyond this shared segment") );
      }
      for ( int i = 0; _nm_table[i].name != -1; ++i ) {
        if ( _nm_table[i].name == name ) {
          throw std::invalid_argument( std::string("name id already exist") );
        }
      }
      _nm_table[_last].name = name;
      _nm_table[_last].offset = reinterpret_cast<const char *>(&obj) - reinterpret_cast<char *>(shm_alloc<_Inst>::_seg.address());
      _nm_table[_last].count = 1;
      ++_last;
    }

    template <class T>
    T& named( int name )
    {
      xmt::basic_lock<__mutex<false,true> > lk( _lock );
      for ( int i = 0; _nm_table[i].name != -1; ++i ) {
        if ( _nm_table[i].name == name ) {
          ++_nm_table[i].count;
          return *reinterpret_cast<T *>(reinterpret_cast<char *>(shm_alloc<_Inst>::_seg.address()) + _nm_table[i].offset );
        }
      }
      throw std::invalid_argument( std::string("name id not found") );
    }

    template <class T>
    const T& named( int name ) const
    {
      xmt::basic_lock<__mutex<false,true> > lk( _lock );
      for ( int i = 0; _nm_table[i].name != -1; ++i ) {
        if ( _nm_table[i].name == name ) {
          ++_nm_table[i].count;
          return *reinterpret_cast<const T *>(reinterpret_cast<char *>(shm_alloc<_Inst>::_seg.address()) + _nm_table[i].offset );
        }
      }
      throw std::invalid_argument( std::string("name id not found") );
    }

    template <class T>
    void release( int name )
    {
      xmt::basic_lock<__mutex<false,true> > lk( _lock );
      for ( int i = 0; _nm_table[i].name != -1; ++i ) {
        if ( _nm_table[i].name == name ) {
          if ( --_nm_table[i].count == 0 ) {
            reinterpret_cast<T *>(reinterpret_cast<char *>(shm_alloc<_Inst>::_seg.address()) + _nm_table[i].offset )->~T();
            // shift table;
            std::copy( _nm_table + i + 1, _nm_table + _last + 1, _nm_table + i );
            --_last;
          }
          return;
        }
      }
      throw std::invalid_argument( std::string("name id not found") );
    }

    int count( int name ) const throw()
    {
      xmt::basic_lock<__mutex<false,true> > lk( _lock );
      for ( int i = 0; _nm_table[i].name != -1; ++i ) {
        if ( _nm_table[i].name == name ) {
          return _nm_table[i].count;
        }
      }
      return 0;
    }

  private:
    shm_name_mgr()
      {
        _nm_table[0].name = -1;
        _last = 0;
      }
    shm_name_mgr( const shm_name_mgr& )
      { }
    shm_name_mgr& operator =( const shm_name_mgr& )
      { return *this; }

    xmt::__mutex<false,true> _lock;
    struct _name_rec
    {
      int name;
      typename shm_name_mgr<_Inst>::size_type offset;
      int count;
      // size
      // type
    };

    _name_rec _nm_table[256];
    int _last;

    friend class shm_alloc<_Inst>;
};


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
        xmt::__mutex<false,true> _lock;
        size_type _nm;
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
      {
         pointer p = _seg.address();

         if ( p != reinterpret_cast<pointer>(-1) && (force || _seg.count() <= 1) ) {
           _master *m = reinterpret_cast<_master *>( _seg.address() );
           (&m->_lock)->~__mutex<false,true>();
           if ( m->_nm != 0 ) {
             reinterpret_cast<shm_name_mgr<_Inst> *>(reinterpret_cast<char *>(p) + m->_nm)->~shm_name_mgr<_Inst>();
           }
         }
         _seg.deallocate( force );
      }

    static size_type max_size() throw()
      { return _seg.max_size() == 0 ? 0 : (_seg.max_size() - sizeof(_master) - sizeof(_aheader)); }

    static shm_name_mgr<_Inst>& name_mgr()
      {
        pointer p = _seg.address();
        if ( p != reinterpret_cast<pointer>(-1) ) {
          _master *m = reinterpret_cast<_master *>( p );
          if (  m->_nm == 0 ) {
            xmt::basic_lock<xmt::__mutex<false,true> > lk( m->_lock );
            void *nm = _traverse( &m->_first, sizeof(shm_name_mgr<_Inst>) );
            m->_nm = reinterpret_cast<char *>(nm) - reinterpret_cast<char *>(p);
            return *new ( nm ) shm_name_mgr<_Inst>();
          }
          return *reinterpret_cast<shm_name_mgr<_Inst> *>(reinterpret_cast<char *>(p) + m->_nm);
        }

        throw shm_bad_alloc( -2 );
      }

  protected:
    static void *allocate( size_type n, void *hint = 0 )
      {
        _master *m = reinterpret_cast<_master *>( _seg.address() );
        if ( m != reinterpret_cast<_master *>(-1) ) {
          xmt::basic_lock<xmt::__mutex<false,true> > lk( m->_lock );
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
        new ( &m._lock ) xmt::__mutex<false,true>();
        xmt::basic_lock<xmt::__mutex<false,true> > lk( m._lock );
        m._first = sizeof( _master );
        m._nm = 0;
        _fheader& h = *new ( reinterpret_cast<char *>(&m) + sizeof(_master) ) _fheader();
        h._next = 0;
        h._sz = _seg.max_size() - sizeof( _master ) - sizeof( _aheader );
      }

  private:
    static const uint64_t MAGIC;
    static detail::__shm_alloc<_Inst> _seg;

    friend class detail::__shm_alloc<_Inst>;
    friend class shm_name_mgr<_Inst>;
};

template <int _Inst>
void shm_alloc<_Inst>::deallocate( pointer p, size_type n )
{
  n = std::max( n + (__align - n % __align) % __align, sizeof(_fheader) );
  _master *m = reinterpret_cast<_master *>( _seg.address() );
  if ( m != reinterpret_cast<_master *>(-1) && (reinterpret_cast<char *>(p) - reinterpret_cast<char *>(_seg.address())) < (_seg.max_size() + sizeof(_master) + sizeof(_aheader) ) ) {
    xmt::basic_lock<xmt::__mutex<false,true> > lk( m->_lock );
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
  n = std::max( n + (__align - n % __align) % __align, sizeof(_fheader) );
  for ( _fheader *h = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + *_prev); *_prev != 0;
        _prev = &h->_next, h = reinterpret_cast<_fheader *>(reinterpret_cast<char *>(_seg.address()) + *_prev)) {
    if ( h->_sz >= (n + sizeof( _fheader )) ) { // reduce this free block, write new free header
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

namespace detail {

template <class T>
class __allocator_shm
{
  private:
    __allocator_shm()
      { }
};

template <>
class __allocator_shm<std::tr1::false_type>
{
  private:
    __allocator_shm()
      { }
};

template <>
class __allocator_shm<std::tr1::true_type>
{
  public:
    __allocator_shm()
      { }
};

#ifndef STLPORT

template <class _Tp, class TRD >
inline void __destroy_aux(_Tp *p, const TRD& /* has_trivial_destructor */)
{ }

template <class _Tp>
inline void __destroy_aux(_Tp *p, const std::tr1::false_type& /* has_trivial_destructor */)
{ p->~_Tp(); }

template <class _Tp>
inline void __destroy_aux(_Tp *p, const std::tr1::true_type& /* has_trivial_destructor */)
{ }

#endif // !STLPORT

} // namespace detail

template <class _Tp, int _Inst>
class allocator_shm :
    public shm_alloc<_Inst>,
    private detail::__allocator_shm<typename is_ipc_sharable<_Tp>::type>
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
      {
#ifdef STLPORT
        _STLP_STD::_Copy_Construct(__p, __val);
#else
        new (__p) _Tp(__val);
#endif
      }

    void destroy(pointer __p)
      {
#ifdef STLPORT
        _STLP_STD::_Destroy(__p);
#else
        detail::__destroy_aux(__p,std::tr1::has_trivial_destructor<value_type>::value);
#endif
      }

};

} // namespace xmt

#endif // __mt_shm_h

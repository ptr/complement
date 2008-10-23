// -*- C++ -*- Time-stamp: <03/11/16 21:49:19 ptr>

/*
 * Copyright (c) 1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
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

#ifndef __node_h
#define __node_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: node.h,v 1.20 2005/03/31 10:22:26 ptr Exp $"
#  else
#pragma ident "@(#)$Id: node.h,v 1.20 2005/03/31 10:22:26 ptr Exp $"
#  endif
#endif

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <list>
#include <string>
#include <map>

#ifndef __XMT_H
#include <mt/xmt.h>
#endif

#include <iostream>
#include <stdexcept>

namespace Helios {

class __node_base
{
  public:
    typedef unsigned key_type;
    typedef std::list<unsigned> grp_container_type;

    enum {
      u_mask   =   00700,
      g_mask   =   00070,
      o_mask   =   00007,
      c_bit    =   01000, // catalog bit
      r_mask   =   00444,
      w_mask   =   00222,
      x_mask   =   00111,
      rw_mask  =   00666
    };

    __node_base() :
        prot( 0 ),
        refcount( 0 ),
        uid( static_cast<unsigned>(-1) ),
        gid( static_cast<unsigned>(-1) )
      { }

    __node_base( unsigned _uid, unsigned _gid, unsigned _prot ) :
        prot( _prot ),
        refcount( 0 ),
        uid( _uid ),
        gid( _gid )
      { }

    unsigned addref()
      { return (unsigned)(++refcount); }
    unsigned rmref()
      { return (unsigned)(--refcount); }
    void protection( unsigned _prot )
      {
        prot &= ~0777;
        prot |= _prot & 0777;
      }
    unsigned protection() const
      { return (unsigned)( prot & 0777 ); }
    bool is_catalog() const
      { return (prot & c_bit) != 0; }

    bool can_read( unsigned __u, unsigned __g ) const
      { return _can( __u, __g, r_mask ); }
    bool can_write( unsigned __u, unsigned __g ) const
      { return _can( __u, __g, w_mask ); }
    bool can_exec( unsigned __u, unsigned __g ) const
      { return _can( __u, __g, x_mask ); }

    bool can_read( unsigned __u, unsigned __g,
                   grp_container_type::const_iterator __first,
                   grp_container_type::const_iterator __last ) const
      { return _can( __u, __g, r_mask, __first, __last ); }
    bool can_write( unsigned __u, unsigned __g,
                    grp_container_type::const_iterator __first,
                    grp_container_type::const_iterator __last ) const
      { return _can( __u, __g, w_mask, __first, __last ); }
    bool can_exec( unsigned __u, unsigned __g,
                   grp_container_type::const_iterator __first,
                   grp_container_type::const_iterator __last ) const
      { return _can( __u, __g, x_mask, __first, __last ); }

    unsigned short prot;
    unsigned short refcount;
    unsigned uid;
    unsigned gid;

  private:
    bool _can( unsigned __u, unsigned __g, unsigned mask ) const
      { return (prot & ( __u == uid ? u_mask : __g == gid ? g_mask : o_mask) & mask) != 0; }
    bool _can( unsigned __u, unsigned __g, unsigned mask,
               grp_container_type::const_iterator __first,
               grp_container_type::const_iterator __last ) const
      {
        if ( (prot & ( __u == uid ? u_mask : __g == gid ? g_mask : o_mask) & mask) != 0 )
          return true;

        if ( (prot & g_mask & mask) != 0 ) { // any group permissions
          while ( __first != __last ) {
            if ( gid == *__first++ ) { // user is group member
              return true;
            }
          }
        }
        return false;
      }
};

class __node_base_catalog :
    public __node_base
{
  public:
    typedef std::pair<key_type,std::string> catalog_entry;
//     typedef __STD::list<catalog_entry, __STL_DEFAULT_ALLOCATOR(catalog_entry) > catalog_type;
    typedef std::list<catalog_entry > catalog_type;
    typedef catalog_type::iterator iterator;
    typedef catalog_type::const_iterator const_iterator;

    __node_base_catalog() :
        __node_base()
      { prot |= c_bit; }

    __node_base_catalog( unsigned _uid, unsigned _gid, unsigned _prot ) :
        __node_base( _uid, _gid, _prot )
      { prot |= c_bit; }

    iterator begin()
      { return catalog.begin(); }
    iterator end()
      { return catalog.end(); }
    const_iterator begin() const
      { return catalog.begin(); }
    const_iterator end() const
      { return catalog.end(); }

    iterator erase( iterator __x )
      { return catalog.erase( __x ); }
    iterator erase( key_type __k )
      {
        iterator i = entry( __k );
        if ( i != catalog.end() ) { 
          return catalog.erase( i );
        }
        return i;
      }
    void push( key_type inode, const std::string& name )
      { catalog.push_back(catalog_entry(inode,name)); }
    iterator insert( iterator i, key_type inode, const std::string& name )
      { return catalog.insert( i, catalog_entry(inode,name)); }
    key_type inode( const std::string& name )
      {
        iterator i = entry( name );
        return i != catalog.end() ? (*i).first : static_cast<key_type>(-1);
      }
    iterator entry( const std::string& name )
      {
        iterator i = catalog.begin();
        while ( i != catalog.end() && (*i).second != name ) {
          ++i;
        }
        return i;
      }
    iterator entry( key_type k )
      {
        iterator i = catalog.begin();
        while ( i != catalog.end() && (*i).first != k ) {
          ++i;
        }
        return i;
      }

    catalog_type::size_type size() const
      { return catalog.size(); }

  protected:
    catalog_type catalog;
};

template <class T>
class __node_base_t :
    public __node_base
{
  public:
    typedef T           value_type;
    typedef value_type& reference;

    __node_base_t() :
        __node_base()
      { }

    __node_base_t( unsigned _uid, unsigned _gid, unsigned _prot ) :
        __node_base( _uid, _gid, _prot )
      { }

    reference value()
      { return data; }

    T data;
};

typedef __node_base_t<std::string> node_string;
typedef __node_base_t<void *> node_void_ptr;

class __nodes_heap_base
{
  public:
    typedef __node_base::key_type key_type;

    __nodes_heap_base() :
        _low( 0 ),
        _high( 0x7fffffff ),
        _id( _low )
      { }

    __nodes_heap_base( key_type __low, key_type __high ) :
        _low( __low ),
        _high( __high ),
        _id( _low )
      { }

    bool is_avail( key_type __id ) const
      { return heap.find( __id ) != heap.end(); }

  protected:
//    typedef __STD::map<key_type,void *,__STD::less<key_type>,
//                                __STL_DEFAULT_ALLOCATOR(void *) > heap_type;

    typedef std::map<key_type,void *,std::less<key_type> > heap_type;
    void *& value( key_type __id )
      {
        // _STLP_ASSERT( heap.find( __id ) != heap.end() );
        return heap[__id];
      }
    key_type create_unique();
    const __impl::MutexSDS& oplock() const
      { return _op_lock; }

    const key_type _low;
    const key_type _high;

    key_type _id;
    heap_type heap;
    __impl::Mutex _lock;
    __impl::MutexSDS _op_lock;
    static const std::string dot;
    static const std::string dotdot;
};

template <class Node> class nodes_heap_cursor;

template <class Node>
class nodes_heap :
    public __nodes_heap_base
{
  public:
    typedef typename Node::reference reference;
    typedef __node_base_catalog Catalog;
    typedef nodes_heap_cursor<Node> cursor_type;

    nodes_heap() :
        __nodes_heap_base()
      { create_root( 0755 ); }

    nodes_heap( unsigned prot ) :
        __nodes_heap_base()
      { create_root( prot ); }

    nodes_heap( key_type __low, key_type __high, unsigned prot = 0755 ) :
        __nodes_heap_base( __low, __high )
      { create_root( prot ); }

    heap_type::size_type size()
      { return heap.size(); }

    reference operator[]( key_type __id )
      {
        // __node_base *tmp = reinterpret_cast<__node_base *>
        // if this is catalog, result undefined
        return reinterpret_cast<Node *>(heap[__id])->value();
      }

    cursor_type cursor( unsigned uid, unsigned gid, unsigned prot_mask = 0 )
      { return cursor_type( this, uid, gid, prot_mask ); }

    key_type create_node( unsigned uid, unsigned gid, unsigned prot );
    key_type create_catalog( unsigned uid, unsigned gid, unsigned prot );
    void destroy( key_type __id );

  private:
    key_type create_root( unsigned );

    friend class nodes_heap_cursor<Node>;
};

class entry_error :
// #ifdef __GNUC__
//    public __STD_QUALIFIER runtime_error
// #else
    public std::runtime_error
// #endif
{
  public:
    entry_error(const std::string& __arg) : runtime_error(__arg) {}
};


template <class Node>
class nodes_heap_cursor
{
  public:
    typedef Helios::__node_base::key_type key_type;
    typedef __node_base_catalog Catalog;
    typedef Catalog::iterator iterator;
    typedef Catalog::const_iterator const_iterator;
    typedef typename Node::value_type value_type;
    typedef typename Node::reference reference;

    nodes_heap_cursor() :
        _heap( 0 ),
        catalog( 0 ),
        _uid( -1 ),
        _gid( -1 ),
        _defprot( 0 )
      { }
    nodes_heap_cursor( const nodes_heap_cursor<Node>& cursor ) :
        _heap( cursor._heap ),
        _uid( cursor._uid ),
        _gid( cursor._gid ),
        _defprot( cursor._defprot ),
        _grp( cursor._grp )
      {
        catalog = cursor.catalog;
        catalog->addref(); // that's locking indeed
      }
    ~nodes_heap_cursor()
      {
        if ( catalog != 0 ) catalog->rmref();
      }

    void close()
      {
        _grp.clear();
        if ( catalog != 0 ) {
          catalog->rmref();
          catalog = 0;
        }
      }

    const __impl::MutexSDS& oplocker()
      {
        if ( _heap == 0 ) {
          throw std::domain_error( "nodes_heap_cursor" );
        }
        return _heap->oplock();
      }

    bool operator ==( const nodes_heap_cursor<Node>& cursor )
      { return _heap == cursor._heap && catalog == cursor.catalog; }
    bool operator !=( const nodes_heap_cursor<Node>& cursor )
      { return _heap != cursor._heap || catalog != cursor.catalog; }

    void cr_uid( unsigned __uid )
      { _uid = __uid; }
    void cr_gid( unsigned __gid )
      { _gid = __gid; }
    void cr_own( unsigned __uid, unsigned __gid )
      { _uid = __uid; _gid = __gid; }
    void add_grp( unsigned __gid )
      { _grp.push_back( __gid ); }

    iterator begin()
      { return catalog->begin(); }
    iterator end()
      { return catalog->end(); }
    const_iterator begin() const
      { return catalog->begin(); }
    const_iterator end() const
      { return catalog->end(); }

    bool is_catalog( const_iterator i ) const
      {
        // __stl_assert( i != catalog->end() );
        //_STLP_ASSERT( (*i).first != -1 );
        //_STLP_ASSERT( _heap->value((*i).first) != 0 );
        return reinterpret_cast<__node_base *>(_heap->value((*i).first))->is_catalog();
      }
    unsigned uid() const
      { return _uid; }
    unsigned gid() const
      { return _gid; }
    unsigned uid( const_iterator i ) const
      { return reinterpret_cast<__node_base *>(_heap->value((*i).first))->uid; }
    unsigned gid( const_iterator i ) const
      { return reinterpret_cast<__node_base *>(_heap->value((*i).first))->gid; }
    unsigned uid( key_type k ) const
      { return uid( catalog->entry( k ) ); }
    unsigned gid( key_type k ) const
      { return gid( catalog->entry( k ) ); }
    unsigned uid( const std::string& name ) const
      { return uid( catalog->entry( name ) ); }
    unsigned gid( const std::string& name ) const
      { return gid( catalog->entry( name ) ); }
    void uid( iterator i, unsigned __uid )
      {
        if ( _uid == 0 || _gid == 0 ) {
          reinterpret_cast<__node_base *>(_heap->value((*i).first))->uid = __uid;
        }
      }
    void gid( iterator i, unsigned __gid )
      {
        if ( _uid == 0 || _gid == 0 ) {
          reinterpret_cast<__node_base *>(_heap->value((*i).first))->gid = __gid;
        } else if ( reinterpret_cast<__node_base *>(_heap->value((*i).first))->uid == _uid ) {
#ifndef __GNUC__
#if !defined(__HP_aCC) || (__HP_aCC > 1)
          typename __node_base::grp_container_type::iterator g = _grp.begin();
#else
          __node_base::grp_container_type::iterator g = _grp.begin();
#endif
#else
//          typename Helios::__node_base::grp_container_type::iterator g = _grp.begin();
          typename std::list<unsigned>::iterator g = _grp.begin();
#endif
          while ( g != _grp.end() ) {
            if ( __gid == *g++ ) {
              reinterpret_cast<__node_base *>(_heap->value((*i).first))->gid = __gid;
              break;
            }
          }
        }
      }
    void uid( key_type k, unsigned __uid )
      { uid( catalog->entry( k ), __uid ); }
    void gid( key_type k, unsigned __gid )
      { gid( catalog->entry( k ), __gid ); }
    void uid( const std::string& name, unsigned __uid )
      { uid( catalog->entry( name ), __uid ); }
    void gid( const std::string& name, unsigned __gid )
      { gid( catalog->entry( name ), __gid ); }
    bool can_read() const
      {
        return reinterpret_cast<__node_base *>(_heap->value(dot()))->can_read( _uid, _gid, _grp.begin(), _grp.end() );
      }
    bool can_write() const
      {
        return reinterpret_cast<__node_base *>(_heap->value(dot()))->can_write( _uid, _gid, _grp.begin(), _grp.end() );
      }
    bool can_exec() const
      {
        return reinterpret_cast<__node_base *>(_heap->value(dot()))->can_exec( _uid, _gid, _grp.begin(), _grp.end() );
      }
    reference operator [] ( iterator i )
      {
        if ( is_catalog(i) ) throw entry_error( "catalog" );        
        return reinterpret_cast<Node *>(_heap->value((*i).first))->value();
      }
    reference operator [] ( const std::string& name )
      {
        iterator i = catalog->entry( name );
        if ( i == catalog->end() ) throw entry_error( "no entry" );
        if ( is_catalog(i) ) throw entry_error( "catalog" );
        return reinterpret_cast<Node *>(_heap->value((*i).first))->value();
      }
    iterator entry( const std::string& name )
      { return catalog->entry( name ); }
    iterator entry( key_type k )
      { return catalog->entry( k ); }
    // key_type inode( const std::string& name )
    //   { return catalog->inode( name ); }

    iterator create( const std::string& name, unsigned prot )
      {
        if ( can_write() ) {
          catalog->push( _heap->create_node( _uid, _gid, prot ), name );
          return --catalog->end();
        }
        return catalog->end();
      }
    iterator create( const std::string& name )
      { return create( name, _defprot ); }
    iterator insert( iterator i, const std::string& name, unsigned prot )
      {
        if ( can_write() ) {
          return catalog->insert( i, _heap->create_node( _uid, _gid, prot ), name );
        }
        return catalog->end();
      }
     iterator insert( iterator i, const std::string& name )
      { return insert( i, name, _defprot ); }

    // if default protection has read bit, for catalog should be set exec bit
    iterator create_catalog( const std::string& name )
      { return create_catalog( name, _defprot |
                               ((_defprot & __node_base::r_mask) >> 2 ) ); }
    iterator create_catalog( const std::string& name, unsigned prot );
    iterator insert_catalog( iterator i, const std::string& name )
      { return insert_catalog( i, name, _defprot |
                               ((_defprot & __node_base::r_mask) >> 2 ) ); }
    iterator insert_catalog( iterator i, const std::string& name, unsigned prot );

    void erase( const std::string& name )
      { erase( catalog->entry( name ) ); }
    void erase( iterator i );
    void erase( key_type k )
      { erase( catalog->entry( k ) ); }

    void cd( key_type k );
    void cd( iterator i );
    void cd( const std::string& name )
      { cd( catalog->entry(name) ); }
    void cd_dotdot()
      { cd( ++begin() ); } 

    iterator mv( iterator i, nodes_heap_cursor<Node>& cursor );
    iterator mv( iterator i, nodes_heap_cursor<Node>& cursor, const std::string& name );
    iterator mv( key_type k, nodes_heap_cursor<Node>& cursor );
    iterator mv( key_type k, nodes_heap_cursor<Node>& cursor, iterator i );
    iterator ln( iterator i );
    iterator ln( key_type k, nodes_heap_cursor<Node>& cursor );

    unsigned protection( iterator i )
      { return reinterpret_cast<__node_base *>(_heap->value((*i).first))->protection(); }
    unsigned protection( key_type k )
      { return protection( catalog->entry( k ) ); }
    unsigned protection( const std::string& name )
      { return protection( catalog->entry( name ) ); }
    void protection( iterator i, unsigned p )
      {
        if ( reinterpret_cast<__node_base *>(_heap->value((*i).first))->uid == _uid || _uid == 0 ) {
          reinterpret_cast<__node_base *>(_heap->value((*i).first))->protection( p );
        }
      }
    void protection( key_type k, unsigned p )
      { protection( catalog->entry( k ), p ); }
    void protection( const std::string& name, unsigned p )
      { protection( catalog->entry( name ), p ); }

    key_type dot() const
      { return (*catalog->begin()).first; }
    key_type dotdot() const
      { return (*++catalog->begin()).first; }

  protected:
    // prot_mask here is mask for clear protection bits from 0666
    // I never set default exec bit. It's set for catalogs if default
    // protection has read bit (see above, create_catalog)

    nodes_heap_cursor( nodes_heap<Node>& __heap, unsigned uid, unsigned gid, unsigned prot_mask ) :
        _heap( &__heap ),
        _uid( uid ),
        _gid( gid ),
        _defprot( ~prot_mask & __node_base::rw_mask ) // clear exec bits
      {
        catalog = reinterpret_cast<Catalog *>(_heap->value(0));
        catalog->addref();
      }
    nodes_heap_cursor( nodes_heap<Node> *__heap, unsigned uid, unsigned gid, unsigned prot_mask ) :
        _heap( __heap ),
        _uid( uid ),
        _gid( gid ),
        _defprot( ~prot_mask & __node_base::rw_mask ) // clear exec bits
      {
        catalog = reinterpret_cast<Catalog *>(_heap->value(0));
        catalog->addref();
      }

    bool erase_catalog( __node_base& node );

    Catalog *catalog;
    nodes_heap<Node> *_heap;
    unsigned _uid;
    unsigned _gid;
    unsigned _defprot;
#if defined(__HP_aCC) && (__HP_aCC == 1)
    __node_base::grp_container_type _grp;
#else
    typename __node_base::grp_container_type _grp;
#endif

    friend class nodes_heap<Node>;
};

typedef nodes_heap<node_string> stringtree;

} // namespace Helios

#ifndef __STL_LINK_TIME_INSTANTIATION
#  include <misc/node.cc>
#endif

#endif // __node.h

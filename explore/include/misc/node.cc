// -*- C++ -*- Time-stamp: <02/08/01 09:58:50 ptr>

/*
 *
 * Copyright (c) 1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

namespace Helios {

template <class Node>
void nodes_heap<Node>::destroy( key_type __id )
{
  MT_REENTRANT( _lock, _x1 );
  heap_type::iterator i = heap.find( __id );
  if ( i != heap.end() ) {
    __node_base& node = *reinterpret_cast<__node_base *>((*i).second);
    if ( node.refcount == 0 ) {
      if ( node.is_catalog() ) { // catalog
        delete reinterpret_cast<Catalog *>((*i).second);
      } else {
        delete reinterpret_cast<Node *>((*i).second);
      }
      heap.erase( i );
    }
  }
}

template <class Node>
__nodes_heap_base::key_type nodes_heap<Node>::create_node( unsigned uid, unsigned gid, unsigned prot )
{
  MT_REENTRANT( _lock, _x1 );

  key_type id = create_unique();
  Node *tmp = new Node( uid, gid, prot );
  heap[id] = tmp;
  tmp->addref();
  return id;
}

template <class Node>
__nodes_heap_base::key_type
nodes_heap<Node>::create_catalog( unsigned uid, unsigned gid, unsigned prot )
{
  MT_REENTRANT( _lock, _x1 );

  key_type id = create_unique();
  Catalog *tmp = new Catalog( uid, gid, prot );
  heap[id] = tmp;
  tmp->addref();
  return id;
}

template <class Node>
__nodes_heap_base::key_type nodes_heap<Node>::create_root( unsigned prot )
{
  MT_REENTRANT( _lock, _x1 );
  Catalog *root = new Catalog( 0, 0, prot );

  heap[0] = root;
  root->push( 0, dot );
  root->addref();
  root->push( 0, dotdot );
  root->addref();

  return 0;
}

template <class Node>
__FIT_TYPENAME nodes_heap_cursor<Node>::iterator
nodes_heap_cursor<Node>::create_catalog( const std::string& name, unsigned prot )
{
  if ( !can_write() ) {
    return catalog->end(); // no permission
  }

  key_type k = _heap->create_catalog( _uid, _gid, prot | ((prot & __node_base::r_mask) >> 2) );
  catalog->push( k, name );
  Catalog *c = reinterpret_cast<Catalog *>(_heap->value( k ));
  c->push( k, _heap->dot );
  c->addref();
  c->push( dot(), _heap->dotdot );
  catalog->addref();
  return --catalog->end();
}

template <class Node>
__FIT_TYPENAME nodes_heap_cursor<Node>::iterator
nodes_heap_cursor<Node>::insert_catalog( __FIT_TYPENAME nodes_heap_cursor<Node>::iterator i,
                                         const std::string& name, unsigned prot )
{
  if ( !can_write() ) {
    return catalog->end(); // no permission
  }

  key_type k = _heap->create_catalog( _uid, _gid, prot | ((prot & __node_base::r_mask) >> 2) );
  i = catalog->insert( i, k, name );
  Catalog *c = reinterpret_cast<Catalog *>(_heap->value( k ));
  c->push( k, _heap->dot );
  c->addref();
  c->push( dot(), _heap->dotdot );
  catalog->addref();
  return i;
}

template <class Node>
void nodes_heap_cursor<Node>::erase( iterator i )
{
  if ( i != catalog->end() ) {
    key_type k = (*i).first;
    if ( !can_write() ) {
      throw entry_error( "permission denied" );
    }
    __node_base& node = *reinterpret_cast<__node_base *>(_heap->value(k));
//    if ( !node.can_write( _uid, _gid, _grp.begin(), _grp.end() ) ) {
//      throw entry_error( "permission denied" );
//    }
    if ( node.is_catalog() && erase_catalog( node ) == false ) {
      throw entry_error( "catalog not empty" );
    }
    node.rmref();
    catalog->erase( i );
    _heap->destroy( k );
  }
}

template <class Node>
bool nodes_heap_cursor<Node>::erase_catalog( __node_base& node )
{
  Catalog& c = *reinterpret_cast<Catalog *>(&node);
  if ( c.size() > 2 ) {
    return false;
  }
  c.erase( c.begin() ); // .
  c.rmref();
  c.erase( c.begin() ); // ..
  catalog->rmref();
  return true;
}

template <class Node>
void nodes_heap_cursor<Node>::cd( key_type k )
{
  iterator i = catalog->entry( k );
  if ( i != catalog->end() ) {
    __node_base& node = *reinterpret_cast<__node_base *>(_heap->value(k));
    if ( node.is_catalog() ) {
      if ( !node.can_read( _uid, _gid, _grp.begin(), _grp.end() ) || !node.can_exec( _uid, _gid, _grp.begin(), _grp.end() ) ) {
        throw entry_error( "permission denied" );
//        return; // no permission
      }
      catalog->rmref();
      catalog = reinterpret_cast<Catalog *>(&node);
      catalog->addref();
    }
  }
}

template <class Node>
void nodes_heap_cursor<Node>::cd( iterator i )
{
  if ( i != catalog->end() ) {
    key_type k = (*i).first;
    __node_base& node = *reinterpret_cast<__node_base *>(_heap->value(k));
    if ( node.is_catalog() ) {
      if ( !node.can_read( _uid, _gid, _grp.begin(), _grp.end() ) || !node.can_exec( _uid, _gid, _grp.begin(), _grp.end() ) ) {
        throw entry_error( "permission denied" );
//        return; // no permission
      }
      catalog->rmref();
      catalog = reinterpret_cast<Catalog *>(&node);
      catalog->addref();
    }
  }
}

template <class Node>
__FIT_TYPENAME nodes_heap_cursor<Node>::iterator
nodes_heap_cursor<Node>::mv( iterator i, nodes_heap_cursor<Node>& cursor )
{
  if ( cursor._heap != _heap ) {
    throw std::domain_error( "different heaps" );
  }
  if ( cursor.catalog == catalog ) {
    throw std::domain_error( "same catalog" );
  }
  if ( !reinterpret_cast<__node_base *>(cursor._heap->value(cursor.dot()))->can_write( _uid, _gid, _grp.begin(), _grp.end() ) ) {
    throw entry_error( "permission denied" );
  }
  if ( !can_write() ) {
    throw entry_error( "permission denied" );
  }

  key_type k = (*i).first;
  cursor.catalog->push( k, (*i).second );
  catalog->rmref();
  catalog->erase( i );
  cursor.catalog->addref();

  return --cursor.catalog->end();
}

template <class Node>
__FIT_TYPENAME nodes_heap_cursor<Node>::iterator
nodes_heap_cursor<Node>::mv( key_type k, nodes_heap_cursor<Node>& cursor )
{
  if ( cursor._heap != _heap ) {
    throw std::domain_error( "different heaps" );
  }
  if ( cursor.catalog == catalog ) {
    throw std::domain_error( "same catalog" );
  }
  if ( !reinterpret_cast<__node_base *>(cursor._heap->value(cursor.dot()))->can_write( _uid, _gid, _grp.begin(), _grp.end() ) ) {
    throw entry_error( "permission denied" );
  }
  if ( !can_write() ) {
    throw entry_error( "permission denied" );
  }

  iterator i = catalog->entry( k );
  if ( i == catalog->end() ) {
    throw entry_error( "no entry" );
  }

  cursor.catalog->push( k, (*i).second );
  catalog->rmref();
  catalog->erase( i );
  cursor.catalog->addref();

  return --cursor.catalog->end();
}

template <class Node>
__FIT_TYPENAME nodes_heap_cursor<Node>::iterator
nodes_heap_cursor<Node>::mv( key_type k, nodes_heap_cursor<Node>& cursor, iterator ic )
{
  if ( cursor._heap != _heap ) {
    throw std::domain_error( "different heaps" );
  }        
  if ( !reinterpret_cast<__node_base *>(cursor._heap->value(cursor.dot()))->can_write( _uid, _gid, _grp.begin(), _grp.end() ) ) {
    throw entry_error( "permission denied" );
  }
  if ( !can_write() ) {
    throw entry_error( "permission denied" );
  }

  iterator i = catalog->entry( k );
  if ( cursor.catalog == catalog && i == ic ) {
    throw std::domain_error( "moving in self" );
  }

  if ( i == catalog->end() ) {
    throw entry_error( "no entry" );
  }

  ic = cursor.catalog->insert( ic, k, (*i).second );
  catalog->rmref();
  catalog->erase( i );
  cursor.catalog->addref();

  return ic;
}

template <class Node>
__FIT_TYPENAME nodes_heap_cursor<Node>::iterator
nodes_heap_cursor<Node>::mv( iterator i, nodes_heap_cursor<Node>& cursor, const std::string& name )
{
  if ( cursor._heap != _heap ) {
    throw std::domain_error( "different heaps" );
  }        
  if ( !reinterpret_cast<__node_base *>(cursor._heap->value(cursor.dot()))->can_write( _uid, _gid, _grp.begin(), _grp.end() ) ) {
    throw entry_error( "permission denied" );
  }
  if ( can_write() ) {
    if ( cursor.catalog != catalog ) {
      key_type k = (*i).first;
      cursor.catalog->push( k, name );
      catalog->rmref();
      catalog->erase( i );
      cursor.catalog->addref();
      return --cursor.catalog->end();
    } else {
      (*i).second = name;
      return i;
    }
  }
  throw entry_error( "permission denied" );
}


template <class Node>
__FIT_TYPENAME nodes_heap_cursor<Node>::iterator nodes_heap_cursor<Node>::ln( iterator i )
{
  if ( is_catalog( i ) ) {
    throw entry_error( "can't link catalog" );
  }
/*
  if ( !reinterpret_cast<__node_base *>(cursor._heap->value(cursor.dot()))->can_write( _uid, _gid, _grp.begin(), _grp.end() ) ) {
    throw entry_error( "permission denied" );
  }
*/
  if ( can_write() ) {
    key_type k = (*i).first;
    __node_base& node = *reinterpret_cast<__node_base *>(_heap->value(k));
    /* cursor. */ catalog->push( k, (*i).second );
    /* cursor. */ catalog->addref();
    node.addref();
    return -- /* cursor. */ catalog->end();
  }
  throw entry_error( "permission denied" );
}

template <class Node>
__FIT_TYPENAME nodes_heap_cursor<Node>::iterator
nodes_heap_cursor<Node>::ln( key_type k, nodes_heap_cursor<Node>& cursor )
{
  iterator i = cursor.entry( k );
  if ( i == cursor.end() ) {
    throw entry_error( "no entry" );
  }
  if ( cursor._heap != _heap ) {
    throw std::domain_error( "different heaps" );
  }
  return this->ln( i );
}

} // namespace Helios

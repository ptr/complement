// -*- C++ -*- Time-stamp: <10/05/13 10:39:44 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

// #include "yard.h"
#include <yard/yard.h>

#include <inttypes.h>
#include <mt/uid.h>
#include <fstream>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <exam/defs.h>

namespace yard {

using namespace std;

const size_t underground::first_hash_size = 0x200; // 0x00 - 0x1ff
const size_t underground::block_size = 4096;
const size_t underground::hash_block_n = (underground::block_size - 2 * sizeof(uint64_t)) / (sizeof(id_type) + 2 * sizeof(uint64_t));
const size_t underground::hash_block_id_off = 2 * sizeof(uint64_t);
const size_t underground::hash_block_off_off = underground::hash_block_id_off + underground::hash_block_n * sizeof(id_type);

// xmt::uuid_type obj_table;

// struct block_basket
// {
//     off_t block_offset[first_hash_size];
// };

// uint64_t block_offset[256];

// struct block_header
// {
//     size_t count;
//     size_t bs;
// };

struct data_descr
{
    off_t  off;
    size_t sz;
};

/*
  Block:

  uint64_t count;
  // uint64_t bs;
  uint64_t next;

  id_type id[count];
  id_type pad[];          <-- 0 + count * sizeof(id_type) + sizeof(uint64_t) * 2
  data_descr data[count]; <-- 0 + bs * sizeof(id_type) + sizeof(uint64_t) * 2
  
*/

underground::underground( const char* path ) :
    f( path, ios_base::in | ios_base::out ),
    hoff( 0 ),
    block_offset( new offset_type[first_hash_size] )
{
  if ( !f.is_open() ) {
    f.clear();
    f.open( path, ios_base::in | ios_base::out | ios_base::trunc );

    hoff = 8 * sizeof(uint64_t); // see below, 8 is a number of sections entries
    uint64_t v = 1;

    // section 1 (hash table) -> hoff
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
    v = hoff;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );

    // section 2 (objects/revisions) -> ...

    v = 2;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
    v = ds = hoff + first_hash_size * sizeof(uint64_t);
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );

    // size of hash (3) -> first_hash_size
    v = 3;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
    v = first_hash_size;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );

    // end of sections
    v = 0;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
    v = 0;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );

    // ds = hoff + first_hash_size * sizeof(offset_type);
    // if ( hoff != 0 ) {
    //   f.seekp( hoff, ios_base::beg );
    // }

    v = static_cast<offset_type>(-1);
    for ( int i = 0; i < first_hash_size; ++i ) {
      block_offset[i] = static_cast<offset_type>(-1);
      f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
      if ( f.fail() ) {
        f.close();
        break;
      }
    }
    f.flush();
  } else {
    uint64_t v = 1, vv;

    for ( ; f.good() && v != 0; ) {
      f.read( reinterpret_cast<char*>(&v), sizeof(uint64_t) );
      f.read( reinterpret_cast<char*>(&vv), sizeof(uint64_t) );
      if ( !f.fail() ) {
        switch ( v ) {
          case 0:
            break;
          case 1:
            hoff = vv;
	    break;
	  case 2:
	    ds = vv;
	    break;
	  case 3:
            // first_hash_size = vv;
	    break;
	}
      }
    }

    int i = 0;

    ds = hoff + first_hash_size * sizeof(uint64_t);
    
    if ( hoff != 0 ) {
      f.seekg( hoff, ios_base::beg );
    }

    for ( offset_type off = 0; f.good() && (i < first_hash_size); ++i, off += sizeof(uint64_t) ) {
      f.read( reinterpret_cast<char*>(&v), sizeof(uint64_t) );
      if ( f.fail() ) {
        f.close();
        break;
      }
      block_offset[i] = v;
    }
  }

  if ( f.is_open() ) {
    f.clear();
    f.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  }
}

underground::~underground()
{
  delete block_offset;
}

void underground::flush()
{
  f.flush();
}

underground::offset_type underground::create_block( int hv ) throw (std::ios_base::failure)
{
  f.seekp( 0, ios_base::end );

  offset_type off = static_cast<offset_type>(f.tellp()) - ds;

  uint64_t v = 0;
  f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // count

  // block size = 
  // v = (4096 - 2 * sizeof(uint64_t)) / (sizeof(id_type) + 2 * sizeof(uint64_t));
  // f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) ); // bs, 127

  // next:
  v = static_cast<uint64_t>(-1);
  f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) );

  v = 0;

  for ( int i = (underground::block_size - 2 * sizeof(uint64_t)) / sizeof(uint64_t); i > 0; --i ) {
    f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) );
  }

  return off;
  // Note: block offset not in hash table yet!
}

id_type underground::put_revision( const void* data, size_type sz ) throw (std::ios_base::failure)
{
  id_type id = xmt::uid_md5( data, sz );
  put_raw( id, data, sz );
  return id;
}

id_type underground::put_object( const id_type& id, const void* data, size_type sz ) throw (std::ios_base::failure)
{
  id_type rid = put_revision( data, sz );

  try {
    std::string obj;
    offset_type off = get_priv( id, obj );

    if ( obj.size() >= sizeof(id_type) ) {
      if ( *reinterpret_cast<const id_type*>(obj.data() + obj.size() - sizeof(id_type)) == rid ) {
        return rid; // not changed
      }
    }

    // obj += std::string( reinterpret_cast<const char*>(&rid), sizeof(id_type) );
    
    f.seekp( 0, ios_base::end );
    // write data
    offset_type off_data = f.tellp();
    f.write( reinterpret_cast<const char *>(obj.data()), obj.size() );
    f.write( reinterpret_cast<const char *>(&rid), sizeof(id_type) );

    // re-write offset of data...
    f.seekp( off, ios_base::beg );

    uint64_t v = off_data;
    f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // offset
    // ... and data size
    v = obj.size() + sizeof(id_type);
    f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // size

    // old offset and size are lost now
  }
  catch ( const std::invalid_argument& ) {
    put_raw( id, &rid, sizeof(id_type) );
  }

  return rid;
}

void underground::put_raw( const id_type& id, const void* data, underground::size_type sz ) throw (std::ios_base::failure)
{
  int hv = 0x1ff & id.u.i[0];

  if ( block_offset[hv] != static_cast<offset_type>(-1) ) {
    id_type rid;
    uint64_t v = block_offset[hv];
    int n;
    offset_type last_block_off;
    do {
      last_block_off = v + ds;
      f.seekg( last_block_off, ios_base::beg );
      f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
      n = v;
      f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) ); // next
      for ( int i = 0; i < n; ++i ) {
        f.read( reinterpret_cast<char *>(&rid), sizeof(id_type) );
        if ( rid == id ) {
          return;
        }
      }
    } while( v != static_cast<uint64_t>(-1) );

    // no such hash, write new

    f.seekp( 0, ios_base::end );
    // write data
    offset_type off = f.tellp();
    f.write( reinterpret_cast<const char *>(data), sz );

    if ( n < hash_block_n ) {
      v = n + 1;
      f.seekp( last_block_off, ios_base::beg );
      f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // count
      // write id
      f.seekp( last_block_off + hash_block_id_off + n * sizeof(id_type), ios_base::beg );
      f.write( reinterpret_cast<const char *>(&id), sizeof(id_type) );

      // write offset of data...
      f.seekp( last_block_off + hash_block_off_off + n * 2 * sizeof(uint64_t), ios_base::beg );
      v = off - ds;
      f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // offset
      // ... and data size
      v = sz;
      f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // size
    } else {
      create_block( hv );
      // write data
      offset_type off = f.tellp();
      f.write( reinterpret_cast<const char *>(data), sz );

      // write counter, I know that it 1 (just create block)
      f.seekp( off - block_size, ios_base::beg );
      uint64_t v = 1;
      f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // count

      // write id
      f.seekp( off - block_size + hash_block_id_off, ios_base::beg );
      f.write( reinterpret_cast<const char *>(&id), sizeof(id_type) );

      // write offset of data...
      f.seekp( off - block_size + hash_block_off_off, ios_base::beg );
      v = off - ds;
      f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // offset
      // ... and data size
      v = sz;
      f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // size

      // write block offset into previous block in chain
      f.seekp( last_block_off + sizeof(uint64_t), ios_base::beg );
      v = off - block_size - ds;
      f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) );
    }
  } else {
    block_offset[hv] = create_block( hv );
    // write data
    offset_type off = f.tellp();
    f.write( reinterpret_cast<const char *>(data), sz );

    // write counter, I know that it 1 (just create block)
    f.seekp( block_offset[hv] + ds, ios_base::beg );
    uint64_t v = 1;
    f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // count

    // write id
    f.seekp( block_offset[hv] + ds + hash_block_id_off, ios_base::beg );
    f.write( reinterpret_cast<const char *>(&id), sizeof(id_type) );

    // write offset of data...
    f.seekp( block_offset[hv] + ds + hash_block_off_off, ios_base::beg );
    v = off - ds;
    // ... and data size
    f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // offset
    v = sz;
    f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) ); // size
    // write block offset into hash table
    f.seekp( hv * sizeof(uint64_t) + hoff, ios_base::beg );
    v = block_offset[hv];
    f.write( reinterpret_cast<const char *>(&v), sizeof(uint64_t) );
  }

  return;
}

std::string underground::get( const id_type& id ) throw (std::ios_base::failure, std::invalid_argument)
{
#if 0
  int hv = 0x1ff & id.u.i[0];
  if ( block_offset[hv] != static_cast<offset_type>(-1) ) {
    id_type rid;
    uint64_t v = block_offset[hv];
    int n;
    offset_type last_block_off;
    do {
      last_block_off = v + ds;
      f.seekg( last_block_off, ios_base::beg );
      f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
      n = v;
      f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) ); // next
      for ( int i = 0; i < n; ++i ) {
        f.read( reinterpret_cast<char *>(&rid), sizeof(id_type) );
        if ( rid == id ) {
          f.seekg( last_block_off + hash_block_off_off + i * 2 * sizeof(uint64_t), ios_base::beg );
          f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
          offset_type d_off = v + ds;
          f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
          size_type sz = v;
          f.seekg( d_off, ios_base::beg );
          std::string ret( sz, char(0) );
          f.read( const_cast<char *>(ret.data()), sz );
          return ret;
        }
      }
    } while( v != static_cast<uint64_t>(-1) );
  }

  throw std::invalid_argument( "not found" );
#else
  std::string ret;
  get_priv( id, ret );
  return ret;
#endif
}

std::pair<std::string,id_type> underground::get_object_r( const id_type& id ) throw (std::ios_base::failure, std::invalid_argument)
{
  std::pair<std::string,id_type> ret;
  get_priv( id, ret.first ); // object's revisions
  
  // extract last revision
  std::copy( ret.first.data() + ret.first.size() - sizeof(id_type), ret.first.data() + ret.first.size(), reinterpret_cast<char*>(&ret.second) );

  get_priv( ret.second, ret.first ); // last object's revision

  return ret;
}

std::string underground::get_object( const id_type& id ) throw (std::ios_base::failure, std::invalid_argument)
{
  std::string ret;
  get_priv( id, ret ); // object's revisions

  id_type rid;
  
  // extract last revision
  std::copy( ret.data() + ret.size() - sizeof(id_type), ret.data() + ret.size(), reinterpret_cast<char*>(&rid) );
  get_priv( rid, ret ); // last object's revision

  return ret;
}

underground::offset_type underground::get_priv( const id_type& id, std::string& ret ) throw (std::ios_base::failure, std::invalid_argument)
{
  int hv = 0x1ff & id.u.i[0];
  if ( block_offset[hv] != static_cast<offset_type>(-1) ) {
    id_type rid;
    uint64_t v = block_offset[hv];
    int n;
    offset_type last_block_off;
    do {
      last_block_off = v + ds;
      f.seekg( last_block_off, ios_base::beg );
      f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
      n = v;
      f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) ); // next
      for ( int i = 0; i < n; ++i ) {
        f.read( reinterpret_cast<char *>(&rid), sizeof(id_type) );
        if ( rid == id ) {
          offset_type ref_offset = last_block_off + hash_block_off_off + i * 2 * sizeof(uint64_t);
          f.seekg( ref_offset, ios_base::beg );
          f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
          offset_type d_off = v + ds;
          f.read( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
          size_type sz = v;
          f.seekg( d_off, ios_base::beg );
          ret.clear();
          ret.assign( sz, char(0) );
          f.read( const_cast<char *>(ret.data()), sz );
          return ref_offset;
        }
      }
    } while( v != static_cast<uint64_t>(-1) );
  }

  throw std::invalid_argument( "not found" );
}

yard::yard( const char* path ) :
    disc( new underground( path ) )
{
}

yard::~yard()
{
  try {
    flush();
  }
  catch ( ... ) {
  }
  delete disc;
}

void yard::add_manifest( const id_type& id )
{
  vertex*& v = g[id];
  if ( v == 0 ) {
    v = new vertex;
    v->type = manifest;
    v->id = id;
    pair<string,id_type> content;
    try {
      content = disc->get_object_r( id );
      v->rid = content.second;
      v->mod_flag = false;

      // extract from content
      stringstream s( content.first );
      id_type idr;
      id_type ridr;
      uint32_t meta;

      uint32_t blob_sz;

      s.read( reinterpret_cast<char*>(&blob_sz), sizeof(uint32_t) );
      if ( !s.fail() ) {
        v->blob.assign( blob_sz, char(0) );
        s.read( const_cast<char*>(v->blob.data()), blob_sz );
      }

      while ( s.good() ) {
        s.read( reinterpret_cast<char*>(&idr), sizeof(id_type) );
        s.read( reinterpret_cast<char*>(&ridr), sizeof(id_type) );
        s.read( reinterpret_cast<char*>(&meta), sizeof(uint32_t) );

        if ( !s.fail() ) {
          graph_type::iterator u = g.find( idr );
          if ( u != g.end() ) {
            v->adj_list.push_back( make_pair( u->second, meta ) );
            if ( ridr != u->second->rid ) {
              v->mod_flag = true;
            }
          }
        }
      }
    }
    catch ( const std::invalid_argument& ) {
      v->mod_flag = true;
    }
  } else if ( !v->mod_flag ) {
    // check edges
    try {
      pair<string,id_type> content;
      content = disc->get_object_r( id );

      // extract from content
      stringstream s( content.first );
      id_type idr;
      id_type ridr;
      uint32_t meta;

      uint32_t blob_sz;

      s.read( reinterpret_cast<char*>(&blob_sz), sizeof(uint32_t) );
      if ( !s.fail() ) {
        v->blob.assign( blob_sz, char(0) );
        s.read( const_cast<char*>(v->blob.data()), blob_sz );
      }

      while ( s.good() ) {
        s.read( reinterpret_cast<char*>(&idr), sizeof(id_type) );
        s.read( reinterpret_cast<char*>(&ridr), sizeof(id_type) );
        s.read( reinterpret_cast<char*>(&meta), sizeof(uint32_t) );

        if ( !s.fail() ) {
          graph_type::iterator u = g.find( idr );
          if ( u != g.end() ) {
            if ( find_if( v->adj_list.begin(), v->adj_list.end(), compose1( bind2nd(equal_to<vertex*>(), u->second), select1st<pair<vertex*,uint32_t> >() ) ) == v->adj_list.end() ) {
              // problem: what about removed edge? v->mod_flag?
              v->adj_list.push_back( make_pair( u->second, meta ) );
            }
            // v->adj_list.push_back( make_pair( u->second, meta ) );
            // if ( ridr != u->second->rid ) {
            //   v->mod_flag = true;
            // }
          } else {
            // if some_flag ...
            vertex*& v_ch = g[idr];
            v_ch = new vertex;
            v_ch->type = manifest; // ? meta?
            v_ch->id = idr;
            v_ch->rid = ridr;
            v_ch->mod_flag = false;
          }
        }
      }
    }
    catch ( const std::invalid_argument& ) {
      v->mod_flag = true;
    }
  }
}

void yard::add_manifest( const id_type& id1, const id_type& id2 )
{
  add_manifest( id2 );
  add_manifest( id1 );

  vertex*& v2 = g[id2];
  vertex*& v = g[id1];

  if ( find_if( v->adj_list.begin(), v->adj_list.end(), compose1( bind2nd(equal_to<vertex*>(), v2), select1st<pair<vertex*,uint32_t> >() ) ) == v->adj_list.end() ) {
    // (v,v2) not found
    //    i) link not loaded
    /*       it loaded, during add_manifest( id1 );
     *       this case not happens here
     */
    //   ii) link not exist
    v->adj_list.push_back( make_pair( v2, 0 ) );
    if ( !v->mod_flag ) {
      /*
       * Load all vertexes directly accessed from v
       * done during add_manifest( id1 );
       */
      v->mod_flag = true;
    } // else all aready here (should be)
  }
}

void yard::add_leaf( const id_type& id, const void* data, yard::size_type sz )
{
  vertex*& v = g[id];
  id_type rid;

  if ( v == 0 ) {
    v = new vertex;
    v->type = leaf;
    v->id = id;
    v->blob.assign( reinterpret_cast<const char*>(data), sz );

    pair<string,id_type> content;
    try {
      content = disc->get_object_r( id );
      uint32_t sz = *reinterpret_cast<const uint32_t*>(content.first.data());
      string blob_check( content.first.data() + sizeof(uint32_t), sz );
      if ( blob_check != v->blob ) {
        stringstream s;
        uint32_t tmp = sz;
        s.write( reinterpret_cast<const char*>(&tmp), sizeof(uint32_t) );
        s.write( reinterpret_cast<const char*>(data), sz );
        // no edges (at least yet)
        v->rid = disc->put_object( id, s.str() );
        v->mod_flag = true;
      } else {
        v->rid = content.second;
        v->mod_flag = false;
      }
    }
    catch ( const std::invalid_argument& ) {
      stringstream s;
      uint32_t tmp = sz;
      s.write( reinterpret_cast<const char*>(&tmp), sizeof(uint32_t) );
      s.write( reinterpret_cast<const char*>(data), sz );
      // no edges (at least yet)
      v->rid = disc->put_object( id, s.str() );
      v->mod_flag = true;
    }
  } else {
    stringstream s;
    uint32_t tmp = sz;
    s.write( reinterpret_cast<const char*>(&tmp), sizeof(uint32_t) );
    s.write( reinterpret_cast<const char*>(data), sz );
    // no edges (at least yet)
    v->blob.assign( reinterpret_cast<const char*>(data), sz );
    id_type rid = disc->put_object( id, s.str() );
    v->mod_flag = rid != v->rid ? true : false;
  }
}

void yard::add_leaf( const id_type& mid, const id_type& id, const void* data, yard::size_type sz )
{
  add_leaf( id, data, sz );
  add_manifest( mid );

  vertex*& v2 = g[id];
  vertex*& v = g[mid];

  if ( find_if( v->adj_list.begin(), v->adj_list.end(), compose1( bind2nd(equal_to<vertex*>(), v2), select1st<pair<vertex*,uint32_t> >() ) ) == v->adj_list.end() ) {
    // (v,v2) not found
    //    i) link not loaded
    /*       it loaded, during add_manifest( mid );
     *       this case not happens here
     */
    //   ii) link not exist
    v->adj_list.push_back( make_pair( v2, 0 ) );
    if ( !v->mod_flag ) {
      /*
       * Load all vertexes directly accessed from v
       * done during add_manifest( mid );
       */
      v->mod_flag = true;
    } // else all aready here (should be)
  }
}

void yard::flush()
{
  std::list<vertex*> Lb;

  for ( graph_type::iterator i = g.begin(); i != g.end(); ++i ) {
    if ( i->second->mod_flag ) {
      Lb.push_back( i->second );
    }
    i->second->adj_list.push_back( make_pair( vertex::pointer_type(0), 0 ) ); // marker
  }

  if ( !Lb.empty() ) {
    // graph g transpose
    for ( graph_type::const_iterator i = g.begin(); i != g.end(); ++i ) {
      for ( vertex::adj_list_type::iterator j = i->second->adj_list.begin(); /* j != i->second->adj_list.end() */ j->first != 0;  ) {
        j->first->adj_list.push_back( make_pair( i->second, j->second ) );
        i->second->adj_list.erase( j++ );
      }
      i->second->adj_list.pop_front(); // remove marker
    }

    // mark black ...
    while ( !Lb.empty() ) {
      vertex* i = Lb.front();
      Lb.pop_front();
      for ( vertex::adj_list_type::iterator j = i->adj_list.begin(); j != i->adj_list.end(); ++j ) {
        if ( !j->first->mod_flag ) {
          Lb.push_back( j->first );
          j->first->mod_flag = true;
        }
      }
    }

    // graph g transpose
    for ( graph_type::const_iterator i = g.begin(); i != g.end(); ++i ) {
      i->second->adj_list.push_back( make_pair( vertex::pointer_type(0), 0 ) ); // marker
    }

    for ( graph_type::const_iterator i = g.begin(); i != g.end(); ++i ) {
      for ( vertex::adj_list_type::iterator j = i->second->adj_list.begin(); /* j != i->second->adj_list.end() */ j->first != 0;  ) {
        j->first->adj_list.push_back( make_pair( i->second, j->second ) );
        i->second->adj_list.erase( j++ );
      }
      i->second->adj_list.pop_front(); // remove marker
    }

    // Bad algo. I know all blacks after first loop.
    // Do I need to store iterators?
    int blacks;
    do {
      blacks = 0;
      for ( graph_type::iterator i = g.begin(); i != g.end(); ++i ) {
        if ( i->second->mod_flag ) {
          vertex::adj_list_type::const_iterator j = i->second->adj_list.begin();
          while ( (j != i->second->adj_list.end()) && !j->first->mod_flag ) {
            ++j;
          }
          if ( j == i->second->adj_list.end() ) {
            stringstream s;
            // calc i's hash
            uint32_t blob_sz = i->second->blob.size();
            s.write( reinterpret_cast<const char*>(&blob_sz), sizeof(uint32_t) );
            if ( blob_sz > 0 ) {
              s.write( i->second->blob.data(), blob_sz );
            }

            for ( j = i->second->adj_list.begin(); j != i->second->adj_list.end(); ++j ) {
              // s.write( &(*j)->rid, sizeof(id_type) );
              s.write( reinterpret_cast<const char*>(&j->first->id), sizeof(id_type) );
              s.write( reinterpret_cast<const char*>(&j->first->rid), sizeof(id_type) );
              s.write( reinterpret_cast<const char*>(&j->second), sizeof(uint32_t) );
            }
            disc->put_object( i->first, s.str() );
            // write i
            i->second->mod_flag = false;
          } else {
            // Lb.push_back( *j );
            ++blacks;
          }
        }
      }
    } while ( blacks > 0 );
  }

  disc->flush();
}

std::string yard::get( const id_type& id )
{
  graph_type::const_iterator r = g.find( id );
  if ( r == g.end() ) {
    pair<string,id_type> content;
    try {
      content = disc->get_object_r( id );

      vertex*& v = g[id];
      v = new vertex;
      v->type = leaf;
      v->id = id;
      stringstream s( content.first );
      uint32_t blob_sz;
      s.read( reinterpret_cast<char *>(&blob_sz), sizeof(uint32_t) );
      if ( !s.fail() ) {
        v->blob.assign( blob_sz, char(0) );
        s.read( const_cast<char*>(v->blob.data()), blob_sz );
      }
      v->rid = content.second;
      v->mod_flag = false;

      return v->blob;
    }
    catch ( const std::invalid_argument& a ) {
      throw a;
    }
  }

  return r->second->blob;
}


} // namespace yard

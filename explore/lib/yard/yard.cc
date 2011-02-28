// -*- C++ -*- Time-stamp: <2011-02-28 19:28:51 ptr>

/*
 *
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <yard/yard.h>

#include "pack.h"

#include <inttypes.h>
#include <mt/uid.h>
#include <fstream>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <misc/md5.h>
#include <exam/defs.h>

#include <algorithm>
#include <functional>
#include <cassert>

#if !defined(STLPORT) && defined(__GNUC__)
#  include <ext/functional>
#endif

#include <queue>
#include <list>
#include <iterator>

namespace yard {

using namespace std;

#if !defined(STLPORT) && defined(__GNUC__)
using __gnu_cxx::select1st;
#endif

void metainfo::set( int key, const std::string& val )
{
  container_type::iterator i = find_if( rec.begin(), rec.end(),
                                        compose1( bind2nd( equal_to<int>(), key ),
                                                  select1st<container_type::value_type>() ) );
  if ( i != rec.end() ) {
    i->second = val;
  } else {
    rec.push_back( make_pair( key, val ) );
  }
}

bool metainfo::is_set( int key )
{
  return find_if( rec.begin(), rec.end(),
                  compose1( bind2nd( equal_to<int>(), key ),
                            select1st<container_type::value_type>() ) ) != rec.end();
}

const std::string& metainfo::get( int key )
{
  static const string empty;

  container_type::const_iterator i = find_if( rec.begin(), rec.end(),
                                              compose1( bind2nd( equal_to<int>(), key ),
                                                        select1st<container_type::value_type>() ) );

  return i != rec.end() ? i->second : empty;
}

revision::revision()
{
}

revision::revision( const char* filename, std::ios_base::openmode mode, std::streamsize block_size ) :
    db( filename, mode, block_size )
{
}

revision::~revision()
{
  if ( db.is_open() ) {
    flush();
  }
}

void revision::open( const char* filename, std::ios_base::openmode mode, std::streamsize block_size )
{
  if ( !db.is_open() ) {
    // Required: r.empty()
    db.open( filename, mode, block_size );
  }
}

void revision::flush()
{
  // walk through r, write modified ...
  for ( auto i = r.begin(); i != r.end(); ++i ) {
    if ( (i->second.flags & revision_node::mod) != 0 ) {
      db.insert( i->first, i->second.content );
      i->second.flags &= ~revision_node::mod;
    }
  }
  db.flush();
}

revision_id_type revision::push( const void* data, size_t sz )
{
  MD5_CTX ctx;
  revision_id_type rid;

  MD5Init( &ctx );
  MD5Update( &ctx, reinterpret_cast<const uint8_t*>(data), sz );
  MD5Final( rid.u.b, &ctx );

  if ( r.find( rid ) != r.end() ) {
    return rid;
  }

  revision_node& node = r[rid];

  node.flags |= revision_node::mod;
  node.content.assign( static_cast<const char*>(data), sz );

  return rid;
}

revision_id_type revision::push( const manifest_type& m )
{
  stringstream s;

  uint32_t sz;

  for ( manifest_type::const_iterator i = m.begin(); i != m.end(); ++i ) {
    sz = i->first.length();
    s.write( reinterpret_cast<const char*>(&sz), sizeof(uint32_t) );
    s.write( i->first.data(), i->first.length() );
    s.write( reinterpret_cast<const char*>(i->second.u.b), sizeof(revision_id_type) );
  }

  return push( s.str() );
}

revision_id_type revision::push( const diff_type& d )
{
  stringstream s;

  uint32_t sz;

  for ( manifest_type::const_iterator i = d.first.begin(); i != d.first.end(); ++i ) {
    sz = i->first.length();
    s.write( reinterpret_cast<const char*>(&sz), sizeof(uint32_t) );
    s.write( i->first.data(), i->first.length() );
    s.write( reinterpret_cast<const char*>(i->second.u.b), sizeof(revision_id_type) );
  }

  // delimiter: based on knowledge that no key of length 0 in manifest
  sz = 0;
  s.write( reinterpret_cast<const char*>(&sz), sizeof(uint32_t) );

  for ( manifest_type::const_iterator i = d.second.begin(); i != d.second.end(); ++i ) {
    sz = i->first.length();
    s.write( reinterpret_cast<const char*>(&sz), sizeof(uint32_t) );
    s.write( i->first.data(), i->first.length() );
    s.write( reinterpret_cast<const char*>(i->second.u.b), sizeof(revision_id_type) );
  }

  return push( s.str() );
}

revision_id_type revision::push( const commit_node& c, const commit_id_type& cid )
{
  if ( r.find( cid ) != r.end() ) {
    return cid;
  }

  stringstream s;

  int16_t dr = static_cast<uint16_t>(c.dref);
  uint16_t len = static_cast<uint16_t>(c.edge_in.size());

  s.write( reinterpret_cast<const char*>(&dr), sizeof(int16_t) );

  if ( c.dref != -1 ) {
    revision_id_type rid = push( *c.delta );
    s.write( reinterpret_cast<const char*>(rid.u.b), sizeof(revision_id_type) );
  } else {
    s.write( reinterpret_cast<const char*>(c.mid.u.b), sizeof(manifest_id_type) );
  }

  s.write( reinterpret_cast<const char*>(&len), sizeof(uint16_t) );

  for ( commit_node::edge_container_type::const_iterator i = c.edge_in.begin(); i != c.edge_in.end(); ++i ) {
    s.write( reinterpret_cast<const char*>(i->u.b), sizeof(commit_id_type) );
  }

  revision_node& node = r[cid];

  node.flags |= revision_node::mod;
  node.content.assign( s.str() );

  return cid;
}

const std::string& revision::get( const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    try {
      // Try to upload from disk
      // string tmp( db[rid] );

      // revision_node& node = r[rid];
      // node.content.swap( tmp );
      i = r.insert( make_pair(rid, revision_node{ 0, db[rid], xmt::nil_uuid } ) ).first;
      // i = r.find( rid );
    }
    catch ( const std::invalid_argument& err ) {
      // r.erase( rid );
      throw std::invalid_argument( "invalid revision" );
    }
  }

  return i->second.content;
}

void revision::get_manifest( manifest_type& m, const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    try {
      // Try to upload from disk
      // revision_node& node = r[rid];
      // node.content = db[rid];
      i = r.insert( make_pair(rid, revision_node{ 0, db[rid], xmt::nil_uuid } ) ).first;
      // i = r.find( rid );
    }
    catch ( const std::invalid_argument& err ) {
      // r.erase( rid );
      throw std::invalid_argument( "invalid revision" );
    }
  }

  istringstream s( i->second.content );

  uint32_t sz;
  string str;
  revision_id_type rev;

  m.clear();

  do {
    s.read( reinterpret_cast<char*>(&sz), sizeof(uint32_t) );
    if ( s.fail() ) { // to avoid str.resize() for bad sz
      break;
    }
    str.resize( sz );
    s.read( const_cast<char*>(str.data()), sz );
    s.read( reinterpret_cast<char*>(rev.u.b), sizeof(revision_id_type) );
    if ( s.fail() ) {
      break;
    }
    m[str] = rev;
  } while ( s.good() );

  if ( (i->second.flags & revision_node::mod) == 0 ) {
    r.erase( i );
  }
}

void revision::get_diff( diff_type& d, const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    try {
      // Try to upload from disk
      // revision_node& node = r[rid];
      // node.content = db[rid];
      i = r.insert( make_pair(rid, revision_node{ 0, db[rid], xmt::nil_uuid } ) ).first;
      // i = r.find( rid );
    }
    catch ( const std::invalid_argument& err ) {
      // r.erase( rid );
      throw std::invalid_argument( "invalid revision" );
    }
  }

  istringstream s( i->second.content );

  uint32_t sz;
  string str;
  revision_id_type rev;

  d.first.clear();
  d.second.clear();

  do {
    s.read( reinterpret_cast<char*>(&sz), sizeof(uint32_t) );
    if ( s.fail() ) { // to avoid str.resize() for bad sz
      break;
    }
    if ( sz == 0 ) {
      break;
    }
    str.resize( sz );
    s.read( const_cast<char*>(str.data()), sz );
    s.read( reinterpret_cast<char*>(rev.u.b), sizeof(revision_id_type) );
    if ( s.fail() ) {
      break;
    }
    d.first[str] = rev;
  } while ( s.good() );

  do {
    s.read( reinterpret_cast<char*>(&sz), sizeof(uint32_t) );
    if ( s.fail() ) { // to avoid str.resize() for bad sz
      break;
    }
    str.resize( sz );
    s.read( const_cast<char*>(str.data()), sz );
    s.read( reinterpret_cast<char*>(rev.u.b), sizeof(revision_id_type) );
    if ( s.fail() ) {
      break;
    }
    d.second[str] = rev;
  } while ( s.good() );

  if ( (i->second.flags & revision_node::mod) == 0 ) {
    r.erase( i );
  }
}

void revision::get_commit( commit_node& c, const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    try {
      // Try to upload from disk
      // revision_node& node = r[rid];
      // node.content = db[rid];
      i = r.insert( make_pair(rid, revision_node{ 0, db[rid], xmt::nil_uuid } ) ).first;
      // i = r.find( rid );
    }
    catch ( const std::invalid_argument& err ) {
      // r.erase( rid );
      throw std::invalid_argument( "invalid revision" );
    }
  }

  istringstream s( i->second.content );

  int16_t dr;

  s.read( reinterpret_cast<char*>(&dr), sizeof(int16_t) );

  if ( !s.fail() ) {
    c.dref = static_cast<int>(dr);

    revision_id_type rid2;
    s.read( reinterpret_cast<char*>(rid2.u.b), sizeof(revision_id_type) );

    if ( c.dref != -1 ) {
      c.delta = new diff_type;
      get_diff( *c.delta, rid2 );
    } else {
      c.mid = rid2;
    }

    uint16_t len;
    s.read( reinterpret_cast<char*>(&len), sizeof(uint16_t) );
    size_t sz = static_cast<size_t>(len);
    while ( !s.fail() && sz-- > 0 ) {
      s.read( reinterpret_cast<char*>(rid2.u.b), sizeof(commit_id_type) );
      if ( !s.fail() ) {
        c.edge_in.push_back( rid2 );
      }
    }
  }

  if ( (i->second.flags & revision_node::mod) == 0 ) {
    r.erase( i );
  }
}

// this is just md5 of zero-length string; it same as mid
// of manifest of root of commits tree
manifest_id_type yard::root_mid = 
{ {0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
   0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e} };

yard::yard()
{
  cached_manifest[root_mid]; // = m;
  c[xmt::nil_uuid].mid = root_mid; // root
  c[xmt::nil_uuid].delta = 0; // root
  c[xmt::nil_uuid].dref = -1;
}

yard::yard( const char* filename, std::ios_base::openmode mode, std::streamsize block_size ) :
    r( filename, mode, block_size )
{
  cached_manifest[root_mid]; // = m;
  c[xmt::nil_uuid].mid = root_mid; // root
  c[xmt::nil_uuid].delta = 0; // root
  c[xmt::nil_uuid].dref = -1;
}

yard::~yard()
{
  if ( r.is_open() ) {
    r.flush();
  }
}

void yard::open( const char* filename, std::ios_base::openmode mode, std::streamsize block_size )
{
  r.open( filename, mode, block_size );
}

void yard::flush()
{
  r.flush();
}

void yard::open_commit_delta( const commit_id_type& base, const commit_id_type& m )
{
  commit_container_type::const_iterator i = c.find( base );

  if ( i == c.end() ) {
    try {
      r.get_commit( c[base], base );
    }
    catch ( const invalid_argument& ) {
      c.erase( base );
      // throw invalid_argument
      return;
    }
    i = c.find( base );
  }

  try {
    if ( cached_manifest.find( i->second.mid ) == cached_manifest.end() ) {
      r.get_manifest( cached_manifest[i->second.mid], i->second.mid );
    }
    cache[m].first = make_pair( base, base );
  }
  catch ( std::invalid_argument& ) {
    cached_manifest.erase( i->second.mid );
    c.erase( base );
    // throw runtime_error( "no such manifest" )
  }
}

void yard::close_commit_delta( const commit_id_type& m )
{
  cache_container_type::iterator i = cache.find( m );

  if ( i == cache.end() ) {
    return;
  }

  if ( (i->second.first.first != i->second.first.second) ) {
    diff_type diff;
    manifest_id_type mid = aggregate_delta( i->second.first.first, diff );
    for ( manifest_type::const_iterator k = i->second.second.first.begin(); k != i->second.second.first.end(); ++k ) {
      diff.second.erase( k->first );
    }
    for ( manifest_type::const_iterator k = i->second.second.second.begin(); k != i->second.second.second.end(); ++k ) {
      diff.second[k->first] = k->second;
    }

    manifest_type manifest( cached_manifest[mid] );
    for ( manifest_type::const_iterator k = diff.first.begin(); k != diff.first.end(); ++k ) {
      manifest.erase( k->first );
    }
    for ( manifest_type::const_iterator k = diff.second.begin(); k != diff.second.end(); ++k ) {
      manifest[k->first] = k->second;
    }
    
    revision_id_type rid = r.push( manifest );
    c[m].mid = rid;
    c[m].delta = 0;
    c[m].dref = -1;

    swap( cached_manifest[rid], manifest );
    c[m].edge_in.push_back( i->second.first.first );
    c[m].edge_in.push_back( i->second.first.second );
  } else if ( i->second.first.first != xmt::nil_uuid ) {
    c[m].mid = c[i->second.first.first].mid; // check that it present?
    c[m].delta = new diff_type;
    swap( *c[m].delta, i->second.second );
    c[m].edge_in.push_back( i->second.first.first );
    c[m].dref = c[m].edge_in.size() - 1;
  } else { // ab ovo
    manifest_type manifest;

    // maninfest empty, so erase not applied here; start from 'add' section:
    for ( manifest_type::const_iterator k = i->second.second.second.begin(); k != i->second.second.second.end(); ++k ) {
      manifest[k->first] = k->second;
    }

    revision_id_type rid = r.push( manifest );
    c[m].mid = rid;
    c[m].delta = 0;
    c[m].dref = -1;
    swap( cached_manifest[rid], manifest );
    c[m].edge_in.push_back( i->second.first.first );
  }

  leafs_container_type::iterator j = find( leaf.begin(), leaf.end(), i->second.first.first );
  if ( j != leaf.end() ) {
    leaf.erase( j );
  }

  if ( i->second.first.first != i->second.first.second ) {
    j = find( leaf.begin(), leaf.end(), i->second.first.second );
    if ( j != leaf.end() ) {
      leaf.erase( j );
    }
  }

  leaf.push_back( i->first );
  cache.erase( i );

  r.push( c[m], m );
}

manifest_id_type yard::aggregate_delta( const commit_id_type& f, diff_type& diff )
{
  const commit_node* comm = &(c.find( f ))->second;
  const commit_node* orig = comm;

  if ( (comm->delta != 0) && (comm->dref >= 0) && !comm->edge_in.empty() ) {
    stack<const commit_node*, list<const commit_node*> > path;
    commit_node::edge_container_type::const_iterator ee;

    do {
      path.push( comm );

      ee = comm->edge_in.begin();
      advance( ee, comm->dref );
      comm = &(c.find( *ee ))->second;
    } while ( (comm->delta != 0) && (comm->dref >= 0) && !comm->edge_in.empty() );

    while ( !path.empty() ) {
      comm = path.top();

      for ( manifest_type::const_iterator k = comm->delta->first.begin(); k != comm->delta->first.end(); ++k ) {
        diff.second.erase( k->first );
      }
      for ( manifest_type::const_iterator k = comm->delta->second.begin(); k != comm->delta->second.end(); ++k ) {
        diff.first.erase( k->first );
      }
      for ( manifest_type::const_iterator k = comm->delta->first.begin(); k != comm->delta->first.end(); ++k ) {
        diff.first[k->first] = k->second;
      }
      for ( manifest_type::const_iterator k = comm->delta->second.begin(); k != comm->delta->second.end(); ++k ) {
        diff.second[k->first] = k->second;
      }

      path.pop();
    }
  }

  return orig->mid;
}

void yard::add( const commit_id_type& id, const std::string& name, const void* data, size_t sz )
{
  cache_container_type::iterator i = cache.find( id );

  if ( i == cache.end() ) {
    return;
  }

  revision_id_type rid = r.push( data, sz );

  i->second.second.second[name] = rid;
}

void yard::del( const commit_id_type& id, const std::string& name )
{
  cache_container_type::iterator i = cache.find( id );

  if ( i == cache.end() ) {
    return;
  }

  i->second.second.second.erase( name );
  i->second.second.first[name];
}

const std::string& yard::get( const commit_id_type& id, const std::string& name ) throw( std::invalid_argument, std::logic_error )
{
  commit_container_type::const_iterator i = c.find( id );

  if ( i == c.end() ) {
    try {
      r.get_commit( c[id], id );
    }
    catch ( const invalid_argument& ) {
      c.erase( id );
      throw std::invalid_argument( "invalid commit" );
    }
    i = c.find( id );
  }

  if ( i->second.dref != -1 ) { // this is a delta
    diff_type diff;
    manifest_id_type mid = aggregate_delta( id, diff ); // base manifest id
    cached_manifest_type::const_iterator j = cached_manifest.find( mid );
    if ( j == cached_manifest.end() ) {
      try {
        r.get_manifest( cached_manifest[mid], mid );
      }
      catch ( std::invalid_argument& ) {
        cached_manifest.erase( mid );
        throw std::runtime_error( "no such manifest" );
      }

      j = cached_manifest.find( mid ); // != cached_manifest.end(), from above
    }
    // j != cached_manifest.end() here
    manifest_type::const_iterator k = diff.second.find( name );
    if ( k != diff.second.end() ) { // in add part
      try {
        return r.get( k->second );
      }
      catch ( const std::invalid_argument& ) {
        throw std::logic_error( "no such revision" );
      }
    }
    k = diff.first.find( name );
    if ( k != diff.first.end() ) { // in del part
      throw std::invalid_argument( "invalid name" );
    }
    k = j->second.find( name );
    if ( k == j->second.end() ) { // absent in manifest
      throw std::invalid_argument( "invalid name" );
    }
    try {
      return r.get( k->second ); // in base manifest
    }
    catch ( const std::invalid_argument& ) {
      throw std::logic_error( "no such revision" );
    }
  }

  // this is manifest without deltas

  cached_manifest_type::const_iterator j = cached_manifest.find( i->second.mid );

  if ( j == cached_manifest.end() ) {
    try {
      r.get_manifest( cached_manifest[i->second.mid], i->second.mid );
    }
    catch ( std::invalid_argument& ) {
      cached_manifest.erase( i->second.mid );
      throw std::runtime_error( "no such manifest" );
    }

    j = cached_manifest.find( i->second.mid ); // != cached_manifest.end(), from above
  }
  // j != cached_manifest.end() here
  manifest_type::const_iterator k = j->second.find( name );

  if ( k == j->second.end() ) {
    throw std::invalid_argument( "invalid name" );
  }

  try {
    return r.get( k->second );
  }
  catch ( const std::invalid_argument& ) {
    throw std::logic_error( "no such revision" );
  }
}

const std::string& yard::get( const std::string& name ) throw( std::invalid_argument, std::logic_error )
{
  if ( leaf.size() != 1 ) {
    if ( leaf.empty() ) {
      throw std::logic_error( "empty commits graph" );
    }
    throw std::logic_error( "more then one head" );
  }

  return get( leaf.front(), name );
}

diff_type yard::diff( const commit_id_type& from, const commit_id_type& to )
{
  commit_container_type::const_iterator i = c.find( from );

  if ( i == c.end() ) {
    try {
      r.get_commit( c[from], from );
    }
    catch ( const invalid_argument& ) {
      c.erase( from );
      throw std::invalid_argument( "invalid commit id" );
    }
    i = c.find( from );
  }

  commit_container_type::const_iterator j = c.find( to );

  if ( j == c.end() ) {
    try {
      r.get_commit( c[to], to );
    }
    catch ( const invalid_argument& ) {
      c.erase( to );
      throw std::invalid_argument( "invalid commit id" );
    }
    j = c.find( to );
  }

  cached_manifest_type::const_iterator mf = cached_manifest.find( i->second.mid );

  if ( mf == cached_manifest.end() ) {
    try {
      r.get_manifest( cached_manifest[i->second.mid], i->second.mid );
    }
    catch ( std::invalid_argument& ) {
      cached_manifest.erase( i->second.mid );
      throw std::runtime_error( "no such manifest" );
    }

    mf = cached_manifest.find( i->second.mid ); // != cached_manifest.end(), from above
  }

  cached_manifest_type::const_iterator mt = cached_manifest.find( j->second.mid );

  if ( mt == cached_manifest.end() ) {
    try {
      r.get_manifest( cached_manifest[j->second.mid], j->second.mid );
    }
    catch ( std::invalid_argument& ) {
      cached_manifest.erase( j->second.mid );
      throw std::runtime_error( "no such manifest" );
    }

    mt = cached_manifest.find( j->second.mid ); // != cached_manifest.end(), from above
  }

  diff_type df, dt, delta;
  manifest_type::const_iterator l;

  if ( mf == mt ) { // or i->second.mid == j->second.mid, same base
    /* manifest_id_type mfrom = */ aggregate_delta( from, df );
    /* manifest_id_type mto = */   aggregate_delta( to, dt );
    /*
    cerr << HERE << ' '
         << df.first.size() << ' '
         << df.second.size() << ' '
         << dt.first.size() << ' '
         << dt.second.size() << endl;
    */
    for ( manifest_type::const_iterator k = df.second.begin(); k != df.second.end(); ++k ) {
      l = dt.second.find( k->first );
      if ( l == dt.second.end() ) { // not added == removed
        delta.first[k->first] = k->second;
      } else if ( l->second != k->second ) { // changed
        delta.first[k->first] = k->second;
        delta.second[k->first] = l->second; // k->first == l->first here
      }
    }
    for ( manifest_type::const_iterator k = df.first.begin(); k != df.first.end(); ++k ) {
      l = dt.first.find( k->first );
      if ( l == dt.first.end() ) { // not removed == added
        delta.second[k->first] = k->second;
      }
    }
    for ( manifest_type::const_iterator k = dt.first.begin(); k != dt.first.end(); ++k ) {
      l = df.first.find( k->first );
      if ( l == df.first.end() ) { // removed
        delta.first[k->first] = k->second;
      }
    }
    for ( manifest_type::const_iterator k = dt.second.begin(); k != dt.second.end(); ++k ) {
      l = df.second.find( k->first );
      if ( l != df.second.end() ) { // added
        if ( l->second != k->second ) {
          delta.first[k->first] = l->second;
          delta.second[k->first] = k->second;
        }
      } else {
        l = mt->second.find( k->first );
        if ( l != mt->second.end() ) {
          delta.first[k->first] = l->second;
        }
        delta.second[k->first] = k->second;
      }
    }

    return delta;
  }

  for ( manifest_type::const_iterator k = mf->second.begin(); k != mf->second.end(); ++k ) {
    l = mt->second.find( k->first );
    if ( l == mt->second.end() ) { // removed
      delta.first[k->first] = k->second;
    } else if ( l->second != k->second ) { // changed
      delta.first[k->first] = k->second;
      delta.second[k->first] = l->second; // k->first == l->first here
    }
  }

  for ( manifest_type::const_iterator k = mt->second.begin(); k != mt->second.end(); ++k ) {
    l = mf->second.find( k->first );
    if ( l == mf->second.end() ) { // added
      delta.second[k->first] = k->second;
    }
  }

  /* manifest_id_type mfrom = */ aggregate_delta( from, df );
  /* manifest_id_type mto = */   aggregate_delta( to, dt );
  for ( manifest_type::const_iterator k = df.first.begin(); k != df.first.end(); ++k ) {
    l = dt.first.find( k->first );
    if ( l != dt.first.end() ) { // removed both left and right
      delta.first.erase( k->first );
    } else {
      l = mt->second.find( k->first );
      if ( l != mt->second.end() ) { // removed in left, present in right
        delta.second[k->first] = l->second;
      }
    }
  }

  for ( manifest_type::const_iterator k = dt.first.begin(); k != dt.first.end(); ++k ) {
    l = df.first.find( k->first );
    if ( l == df.first.end() ) { // not erased in left
      l = mt->second.find( k->first );
      if ( l != mt->second.end() ) { // was in base manifest in right
        delta.first[k->first] = l->second; // mark as erased
      }
    }
    l = df.second.find( k->first );
    if ( l != df.second.end() ) { // added in left
      delta.first[k->first] = l->second; // mark as erased
    }
  }

  for ( manifest_type::const_iterator k = df.second.begin(); k != df.second.end(); ++k ) {
    l = dt.first.find( k->first );
    if ( l != dt.first.end() ) { // added in left, erased in right
      delta.first[k->first] = l->second; // mark as erased
    }
    l = dt.second.find( k->first );
    if ( l != dt.second.end() ) { // 
      if ( l->second != k->second ) {
        delta.first[k->first] = k->second;
        delta.second[k->first] = l->second;
      } else {
        delta.first.erase( k->first );
        delta.second.erase( k->first );
      }
    } else {
      l = mt->second.find( k->first );
      if ( l != mt->second.end() ) {
        if ( l->second != k->second ) {
          delta.first[k->first] = k->second;
          delta.second[k->first] = l->second;
        } else {
          delta.first.erase( k->first );
          delta.second.erase( k->first );
        }
      } else {
        delta.first[k->first] = k->second;
      }
    }
  }

  for ( manifest_type::const_iterator k = dt.second.begin(); k != dt.second.end(); ++k ) {
    l = df.second.find( k->first );
    if ( l != df.second.end() ) {
      if ( l->second == k->second ) {
        delta.first.erase( k->first );
        delta.second.erase( k->first );
      } else {
        delta.first[k->first] = l->second;
        delta.second[k->first] = k->second;
      }
    } else {
      l = mf->second.find( k->first );
      if ( l != mf->second.end() ) {
        if ( l->second != k->second ) {
          delta.first[k->first] = l->second;
          delta.second[k->first] = k->second;
        } else {
          delta.first.erase( k->first );
          delta.second.erase( k->first );
        }
      } else {
        delta.second[k->first] = k->second;
      }
    }
  }

  return delta;
}

commit_id_type yard::common_ancestor( const commit_id_type& left, const commit_id_type& right )
{
  commit_container_type::iterator l = c.find( left );

  if ( l == c.end() ) {
    try {
      r.get_commit( c[left], left );
    }
    catch ( const invalid_argument& ) {
      c.erase(left);
      throw invalid_argument( "no such commit" );
    }
    l = c.find( left );
  }

  commit_container_type::iterator r = c.find( right );

  if ( r == c.end() ) {
    try {
      yard::r.get_commit( c[right], right );
    }
    catch ( const invalid_argument& ) {
      c.erase(right);
      throw invalid_argument( "no such commit" );
    }
    r = c.find( right );
  }

  if ( left == right ) {
    return left;
  }

  std::list<commit_id_type> colored;

  l->second.color = commit_node::black;
  r->second.color = commit_node::gray;

  colored.push_back( left );
  colored.push_back( right );

  std::queue<commit_id_type,std::list<commit_id_type> > st_left;
  std::queue<commit_id_type,std::list<commit_id_type> > st_right;

  for ( ; ; ) {
    /* This is not infinite loop: graph has single root,
       so at least root is common ancestor
     */
    if ( l != c.end() ) {
      for ( commit_node::edge_container_type::iterator i = l->second.edge_in.begin(); i != l->second.edge_in.end(); ++i ) {
        if ( c[*i].color == commit_node::gray ) { // see right's color
          // clear colors, before return
          for ( std::list<commit_id_type>::const_iterator j = colored.begin(); j != colored.end(); ++j ) {
            c[*j].color = commit_node::white;
          }
          return *i;
        } else if ( c[*i].color == commit_node::white ) {
          c[*i].color = commit_node::black;
          st_left.push( *i );
          colored.push_back( *i );
        } // else already looks here, skip it
      }
      if ( !st_left.empty() ) {
        l = c.find( st_left.front() );
        st_left.pop();
      } else { // root
        l = c.end();
      }
    }

    if ( r != c.end() ) {
      for ( commit_node::edge_container_type::iterator i = r->second.edge_in.begin(); i != r->second.edge_in.end(); ++i ) {
        if ( c[*i].color == commit_node::black ) { // see left's color
          // clear colors, before return
          for ( std::list<commit_id_type>::const_iterator j = colored.begin(); j != colored.end(); ++j ) {
            c[*j].color = commit_node::white;
          }
          return *i;
        } else if ( c[*i].color == commit_node::white ) {
          c[*i].color = commit_node::gray;
          st_right.push( *i );
          colored.push_back( *i );
        } // else already looks here, skip it
      }
      if ( !st_right.empty() ) {
        r = c.find( st_right.front() );
        st_right.pop();
      } else { // root
        r = c.end();
      }
    }
  }
}

int yard::fast_merge( const commit_id_type& merge, const commit_id_type& left, const commit_id_type& right )
{
  commit_id_type ancestor = common_ancestor( left, right );

  if ( ancestor == left ) {
    return 2;
  }

  if ( ancestor == right ) {
    return 1;
  }

  diff_type ld = diff( ancestor, left );
  diff_type rd = diff( ancestor, right );

  for ( manifest_type::iterator i = ld.second.begin(); i != ld.second.end(); ) {
    manifest_type::iterator j = rd.second.find( i->first );
    if ( j != rd.second.end() ) {
      if ( j->second != i->second ) {
        return 3; // can't do fast merge
      }
      ld.second.erase( i++ );
      rd.second.erase( j );
    } else {
      ++i;
    }
  }

  for ( manifest_type::iterator i = ld.first.begin(); i != ld.first.end(); ) {
    manifest_type::iterator j = rd.first.find( i->first );
    if ( j != rd.first.end() ) {
      if ( j->second != i->second ) {
        return 4; // unexpected: shouldn't happen!
      }
      if ( rd.second.find( i->first ) != rd.second.end() ) { // erased in left, changed in right
        return 3; // can't do fast merge
      }
      if ( ld.second.find( i->first ) != ld.second.end() ) { // erased in right, changed in left
        return 3; // can't do fast merge
      }
      ld.first.erase( i++ );
      rd.first.erase( j );
    } else {
      ++i;
    }
  }

  commit_container_type::const_iterator k = c.find( left );

  if ( k == c.end() ) {
    try {
      r.get_commit( c[left], left );
    }
    catch ( const invalid_argument& ) {
      c.erase(left);
      throw invalid_argument( "no such commit" );
    }
    k = c.find( left );
  }

  cache.insert( make_pair( merge, make_pair( make_pair( left, right ), rd ) ) );
  close_commit_delta( merge );

  return 0;
}

int yard::merge( const commit_id_type& merge_, const commit_id_type& left, const commit_id_type& right, conflicts_list_type& cnf )
{
  commit_id_type ancestor = common_ancestor( left, right );

  if ( ancestor == left ) {
    return 2;
  }

  if ( ancestor == right ) {
    return 1;
  }

  diff_type ld = diff( ancestor, left );
  diff_type rd = diff( ancestor, right );

  for ( manifest_type::iterator i = ld.second.begin(); i != ld.second.end(); ) {
    manifest_type::iterator j = rd.second.find( i->first );
    if ( j != rd.second.end() ) {
      if ( j->second != i->second ) { // can't do fast merge
        cnf.push_back( make_pair( i->first, make_pair(i->second,j->second) ) );
      }
      ld.second.erase( i++ );
      rd.second.erase( j );
    } else {
      ++i;
    }
  }

  for ( manifest_type::iterator i = ld.first.begin(); i != ld.first.end(); ) {
    manifest_type::iterator j = rd.first.find( i->first );
    if ( j != rd.first.end() ) {
      if ( j->second != i->second ) {
        return 4; // unexpected: shouldn't happen!
      }
      if ( rd.second.find( i->first ) != rd.second.end() ) { // erased in left, changed in right
        cnf.push_back( make_pair( i->first, make_pair(xmt::nil_uuid,j->second) ) );
      } else if ( ld.second.find( i->first ) != ld.second.end() ) { // erased in right, changed in left
        cnf.push_back( make_pair( i->first, make_pair(i->second,xmt::nil_uuid) ) );
      }
      ld.first.erase( i++ );
      rd.first.erase( j );
    } else {
      ++i;
    }
  }

  commit_container_type::const_iterator k = c.find( left );

  if ( k == c.end() ) {
    try {
      commit_container_type::iterator p;
      commit_node& node = c[left];
      r.get_commit( node, left );
    }
    catch ( const invalid_argument& ) {
      c.erase( left );
      throw runtime_error( "no commit" );
    }
    k = c.find( left );
  }

  for ( manifest_type::iterator i = rd.first.begin(); i != rd.first.end(); ++i ) {
    ld.first[i->first] = i->second;
  }

  for ( manifest_type::iterator i = rd.second.begin(); i != rd.second.end(); ++i ) {
    ld.second[i->first] = i->second;
  }

  cache.insert( make_pair( merge_, make_pair( make_pair( left, right ), ld ) ) );

  return 0;
}

} // namespace yard

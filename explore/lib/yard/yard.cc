// -*- C++ -*- Time-stamp: <2011-01-19 20:32:18 ptr>

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
#include <misc/md5.h>
#include <exam/defs.h>

#include <algorithm>
#include <functional>

#if !defined(STLPORT) && defined(__GNUC__)
#  include <ext/functional>
#endif

#include <queue>
#include <stack>
#include <list>

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

const std::string& revision::get( const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    throw std::invalid_argument( "invalid revision" );
  }

  return i->second.content;
}

void revision::get_manifest( manifest_type& m, const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    throw std::invalid_argument( "invalid revision" );
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
}

void revision::get_diff( diff_type& d, const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    throw std::invalid_argument( "invalid revision" );
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
}

yard_ng::yard_ng()
{
  manifest_type m;
  
  manifest_id_type mid = r.push( m ); // ToDo: clear mod flag in r

  cached_manifest[mid]; // = m;
  c[xmt::nil_uuid].mid = mid; // root
  c[xmt::nil_uuid].delta = 0; // root
  c[xmt::nil_uuid].dref = -1;
}

void yard_ng::open_commit_delta( const commit_id_type& base, const commit_id_type& m )
{
  commit_container_type::const_iterator i = c.find( base );

  if ( i == c.end() ) {
    // ToDo: try to upload from disc
    // throw invalid_argument
    return;
  }

  cache[m].first = make_pair( base, base );
  cached_manifest_type::const_iterator j = cached_manifest.find( i->second.mid );
  if ( j != cached_manifest.end() ) {
    // cache[m].second = j->second; // oh, copy whole manifest...
  } else {
    // cache[m].second = ; extract manifest from r.get(i->second)
  }
}

void yard_ng::close_commit_delta( const commit_id_type& m )
{
  cache_container_type::iterator i = cache.find( m );

  if ( i == cache.end() ) {
    return;
  }

  if ( (i->second.first.first != i->second.first.second) /* ||
                                                            !c[i->second.first.first].edge_out.empty() */ ) {
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
    c[i->second.first.first].edge_out.push_back( m );
    c[i->second.first.second].edge_out.push_back( m );
  } else if ( i->second.first.first != xmt::nil_uuid ) {
    c[m].mid = c[i->second.first.first].mid; // check that it present?
    c[m].delta = new diff_type;
    swap( *c[m].delta, i->second.second );
    c[m].edge_in.push_back( i->second.first.first );
    c[m].dref = c[m].edge_in.size() - 1;
    c[i->second.first.first].edge_out.push_back( m );
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
    c[i->second.first.first].edge_out.push_back( m );
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
}

manifest_id_type yard_ng::aggregate_delta( const commit_id_type& f, diff_type& diff )
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

void yard_ng::add( const commit_id_type& id, const std::string& name, const void* data, size_t sz )
{
  cache_container_type::iterator i = cache.find( id );

  if ( i == cache.end() ) {
    return;
  }

  revision_id_type rid = r.push( data, sz );

  i->second.second.second[name] = rid;
}

void yard_ng::del( const commit_id_type& id, const std::string& name )
{
  cache_container_type::iterator i = cache.find( id );

  if ( i == cache.end() ) {
    return;
  }

  i->second.second.second.erase( name );
  i->second.second.first[name];
}

const std::string& yard_ng::get( const commit_id_type& id, const std::string& name ) throw( std::invalid_argument, std::logic_error )
{
  commit_container_type::const_iterator i = c.find( id );

  if ( i == c.end() ) {
    // ToDo: try to upload from disc
    throw std::invalid_argument( "invalid commit" );
  }

  if ( i->second.dref != -1 ) {
    diff_type diff;
    manifest_id_type mid = aggregate_delta( id, diff );
    cached_manifest_type::const_iterator j = cached_manifest.find( mid );
    if ( j != cached_manifest.end() ) {
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
    } else {
      // deserialise from r.get( i->second ).content;
      cerr << HERE << endl;
    }
  }

  cached_manifest_type::const_iterator j = cached_manifest.find( i->second.mid );

  if ( j != cached_manifest.end() ) {
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
  } else {
    // deserialise from r.get( i->second ).content;
    cerr << HERE << endl;
  }
}

const std::string& yard_ng::get( const std::string& name ) throw( std::invalid_argument, std::logic_error )
{
  if ( leaf.size() != 1 ) {
    if ( leaf.empty() ) {
      throw std::logic_error( "empty commits graph" );
    }
    throw std::logic_error( "more then one head" );
  }

  return get( leaf.front(), name );
}

diff_type yard_ng::diff( const commit_id_type& from, const commit_id_type& to )
{
  commit_container_type::const_iterator i = c.find( from );

  if ( i == c.end() ) {
    // ToDo: try to upload from disc
    throw std::invalid_argument( "invalid commit id" );
  }

  commit_container_type::const_iterator j = c.find( to );

  if ( j == c.end() ) {
    // ToDo: try to upload from disc
    throw std::invalid_argument( "invalid commit id" );
  }

  cached_manifest_type::const_iterator mf = cached_manifest.find( i->second.mid );

  if ( mf == cached_manifest.end() ) {
    cerr << HERE << endl;
  }

  cached_manifest_type::const_iterator mt = cached_manifest.find( j->second.mid );

  if ( mt == cached_manifest.end() ) {
    cerr << HERE << endl;
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

commit_id_type yard_ng::common_ancestor( const commit_id_type& left, const commit_id_type& right )
{
  commit_container_type::iterator l = c.find( left );

  if ( l == c.end() ) {
    throw invalid_argument( "no such commit" );
  }

  commit_container_type::iterator r = c.find( right );

  if ( r == c.end() ) {
    throw invalid_argument( "no such commit" );
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

int yard_ng::fast_merge( const commit_id_type& merge, const commit_id_type& left, const commit_id_type& right )
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
    // ToDo: try to upload from disc
    // throw invalid_argument
    // return;
  }

  cache.insert( make_pair( merge, make_pair( make_pair( left, right ), rd ) ) );
  close_commit_delta( merge );

  return 0;
}

int yard_ng::merge( const commit_id_type& merge_, const commit_id_type& left, const commit_id_type& right, conflicts_list_type& cnf )
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
    // ToDo: try to upload from disc
    // throw invalid_argument
    // return;
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

const size_t underground::block_size = 4096;
const size_t underground::hash_block_n = (underground::block_size - 2 * sizeof(uint64_t)) / (sizeof(id_type) + 2 * sizeof(uint64_t));
const size_t underground::hash_block_id_off = 2 * sizeof(uint64_t);
const size_t underground::hash_block_off_off = underground::hash_block_id_off + underground::hash_block_n * sizeof(id_type);

// xmt::uuid_type obj_table;

// struct block_basket
// {
//     off_t block_offset[hsz];
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
    hsz( 0x200 ), // 0x00 - 0x1ff; must be 'all low bits zero'
    block_offset( 0 )
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
    v = ds = hoff + hsz * sizeof(uint64_t);
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );

    // size of hash (3) -> hsz
    v = 3;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
    v = hsz;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );

    // end of sections
    v = 0;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );
    v = 0;
    f.write( reinterpret_cast<char *>(&v), sizeof(uint64_t) );

    block_offset = new offset_type[hsz];

    v = static_cast<offset_type>(-1);
    for ( int i = 0; i < hsz; ++i ) {
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
            hsz = vv;
	    break;
	}
      }
    }

    int i = 0;

    ds = hoff + hsz * sizeof(uint64_t);
    
    if ( hoff != 0 ) {
      f.seekg( hoff, ios_base::beg );
    }

    block_offset = new offset_type[hsz];

    for ( offset_type off = 0; f.good() && (i < hsz); ++i, off += sizeof(uint64_t) ) {
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

    uint64_t v = off_data - ds;
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
  int hv = (hsz - 1) & id.u.i[0]; // hsz: all low bits must be zero!

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
  int hv = (hsz - 1) & id.u.i[0];
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
  int hv = (hsz - 1) & id.u.i[0];
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

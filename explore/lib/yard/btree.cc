// -*- C++ -*- Time-stamp: <2011-03-16 17:25:22 ptr>

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
#include <exam/defs.h>

#include <algorithm>
#if !defined(STLPORT) && defined(__GNUC__) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
// for copy_n
# include <ext/algorithm>
#endif
#include <functional>
#include <cassert>

#if !defined(STLPORT) && defined(__GNUC__)
#  include <ext/functional>
#endif

#include <iterator>

#include <stdexcept>

namespace yard {

using namespace std;

#if !defined(STLPORT) && defined(__GNUC__) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
using __gnu_cxx::copy_n;
#endif

void write_data( std::fstream& file, BTree::off_type address, const char* data, std::streamsize size )
{
  file.seekp( address );
  file.write( data, size );
}

const BTree::key_type BTree::upper_key_bound = { {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff} };
const BTree::key_type BTree::lower_key_bound = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} };
const std::pair<BTree::key_type,BTree::key_type> BTree::keys_range = { BTree::lower_key_bound, BTree::upper_key_bound };
const BTree::key_type BTree::zero_key = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} };
const uint64_t BTree::magic_id = 0x81a5c36900000000LL; // fix: LSB/MSB
const std::streamsize BTree::alignment = 4096;

BTree::BTree()
{
}

BTree::BTree( const char* filename, std::ios_base::openmode mode, std::streamsize block_size )
{
  BTree::open( filename, mode, block_size );
}

BTree::~BTree()
{
  BTree::close();
}

BTree::off_type BTree::seek_to_end()
{
  file_.seekp( 0, ios_base::end );
  BTree::off_type address = file_.tellp();

  int over = address % alignment;
  BTree::off_type delta = over ? alignment - over : 0;
  file_.seekp( delta, ios_base::end );

  return address + delta;
}

BTree::off_type BTree::append_data( const char* data, std::streamsize size )
{
  off_type address = seek_to_end();
  write_data( file_, address, data, size );

  return address;
}

BTree::off_type BTree::append(const detail::block_type& block)
{
  off_type address = seek_to_end();
  block.pack(file_);
  return address;
}

void BTree::save(off_type offset, const detail::block_type& block)
{
  file_.seekp(offset, ios_base::beg);
  block.pack(file_);
}

void BTree::load( off_type offset, detail::block_type& block ) const
{
  file_.seekg( offset, ios_base::beg );
  block.unpack(file_);
}

const detail::block_type& BTree::get_block( off_type offset ) const
{
  map<off_type, detail::block_type>::const_iterator it = cache_.find(offset);

  if ( it == cache_.end() ) {
    detail::block_type& result = cache_[offset];
    result.set_block_size( bsz );
    load(offset, result);
    return result;
  }

  return it->second;
}

BTree::key_type BTree::min_in_subtree( off_type block_address ) const
{
  const detail::block_type& block = get_block( block_address );

  assert(block.begin() != block.end());

  if ( block.is_leaf() ) {
    return block.begin()->first;
  }

  return min_in_subtree( block.begin()->second.address );
}

BTree::key_type BTree::max_in_subtree( off_type block_address ) const
{
  const detail::block_type& block = get_block( block_address );

  assert(block.begin() != block.end());

  if ( block.is_leaf() ) {
    return (--block.end())->first;
  }

  return max_in_subtree( (--block.end())->second.address );
}

BTree::coordinate_type BTree::lookup(const coordinate_type& start, const key_type& key) const
{
  if ( !is_open() || fail() ) {
    throw ios_base::failure( "BTree io operation fail" );
  }

  if ( start.empty() ) {
    return lookup(key);
  }

  coordinate_type result = start;
  pair<key_type, key_type>& segment = result.top().second;

  while ( (segment.first > key) || (segment.second < key) ) {
    result.pop();

    assert(!result.empty());
    segment = result.top().second;
  }

  lookup_down(result, key);
  return result;
}

/*
BTree::key_type prev_uuid(BTree::key_type key)
{
  if (key.u.l[1] == 0)
    --key.u.l[0];

  --key.u.l[1];
  return key;
}
*/

void BTree::lookup_down( coordinate_type& path, const key_type& key ) const
{
  auto prev_uuid = []( key_type key ) { if ( key.u.l[1]-- == 0 ) { --key.u.l[0]; } return key; };

  for ( ; ;  ) {
    const detail::block_type& new_block = get_block( path.top().first );

    if ( new_block.is_leaf() ) {
      return;
    }

    std::pair<key_type, key_type> segment;

    detail::block_type::const_iterator next_block = new_block.route(key);
    segment.first = next_block->first;

    detail::block_type::const_iterator next_next_block = next_block;
    advance(next_next_block, 1);

    segment.second = next_next_block == new_block.end() ? upper_key_bound : prev_uuid(next_next_block->first);

    path.push(make_pair(next_block->second.address, segment));
  }
}

BTree::coordinate_type BTree::lookup( const key_type& key ) const
{
  if ( !is_open() || fail() ) {
    throw ios_base::failure( "BTree io operation fail" );
  }

  coordinate_type path;
  path.push( make_pair( root_block_off, keys_range ) );

  lookup_down( path, key );

  return path;
}

BTree::key_type BTree::get_shortest_key(const key_type& first_key, const key_type& second_key)
{
    assert(second_key > first_key);
    //chose the key with most zeros at the end
    uint8_t first[16];
    uint8_t second[16];
    uuid_to_long_number(first_key, first);
    uuid_to_long_number(second_key, second);

    uint8_t result[16];
    for (int i = 0; i < 16; ++i)
    {
        result[i] = 0;
    }

    for (int i = 15; i >= 0; --i)
    {
        result[i] = second[i];
        if (second[i] > first[i])
            break;
    }

    key_type new_key;
    long_number_to_uuid(result, new_key);

    assert(new_key <= second_key);
    assert(new_key > first_key);
    return new_key;
}

BTree::coordinate_type BTree::insert(coordinate_type path, const key_type& key, const detail::block_coordinate& coord)
{
  const block_desc& desc = path.top();

  detail::block_type& block = const_cast<detail::block_type&>( get_block( desc.first ) );

  if ( !block.insert(key, coord) ) {
    return path; // key already present
  }

  if ( !block.is_overfilled() ) {
    save(path.top().first, block);
    return path;
  }

  detail::block_type new_block;
  block.divide(new_block);

  pair<xmt::uuid_type, xmt::uuid_type> delimiter;
  if (block.is_leaf()) {
    delimiter.first = (--block.end())->first;
    delimiter.second = new_block.begin()->first;
  } else {
    delimiter.first = max_in_subtree((--block.end())->second.address);
    delimiter.second = min_in_subtree(new_block.begin()->second.address);
  }

  off_type address_of_block = append(block);
  off_type address_of_new_block = append(new_block);

  cache_[address_of_block] = block;
  cache_[address_of_new_block] = new_block;

  cache_.erase(desc.first);

  key_type new_key = get_shortest_key(delimiter.first, delimiter.second);

  detail::block_coordinate new_coord;
  new_coord.address = address_of_new_block;
  new_coord.size = bsz;

  key_type key_to_update = desc.second.first;

  path.pop();

  if ( !path.empty() ) {
    detail::block_type& parent_block = const_cast<detail::block_type&>( get_block( path.top().first ) );

    int count = parent_block.erase(key_to_update);
    assert(count == 1);

    detail::block_coordinate coord;
    coord.address = address_of_block;
    coord.size = bsz;

    bool result = parent_block.insert(key_to_update, coord);
    assert(result);

    return insert(path, new_key, new_coord);
  }

  detail::block_type new_root;
  new_root.set_block_size( bsz );
  new_root.set_flags(detail::block_type::root_node);

  {
    // detail::block_coordinate zero_coord;
    // zero_coord.address = address_of_block;
    // zero_coord.size = bsz;

    bool result = new_root.insert(zero_key, /* zero_coord */ detail::block_coordinate{address_of_block, bsz} );
    assert(result);
  }

  bool result = new_root.insert(new_key, new_coord);
  assert(result);
  cache_[ root_block_off ] = new_root;

  save( root_block_off, new_root);

  return path;
}

void BTree::open( const char* filename, std::ios_base::openmode mode, std::streamsize block_size )
{
  if ( (mode & std::ios_base::trunc) ) {
    file_.open(filename, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

    format_ver = 0;
    bsz = block_size;
    root_block_off = alignment;

    uint8_t  toc_key   = magic;
    uint64_t toc_value = magic_id;
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    toc_key = ver;
    toc_value = format_ver;
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    toc_key = defbsz;
    toc_value = block_size;
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    toc_key = rb_off;
    toc_value = root_block_off;
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    toc_key = toc_reserve;
    toc_value = cglbuf_beg = (std::max)(static_cast<std::streambuf::off_type>(static_cast<std::streambuf::off_type>(file_.tellp()) + (sizeof(toc_key) + sizeof(toc_value)) * 4), static_cast<std::streambuf::off_type>(128) );
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    toc_key = cgleaf_off;
    // toc_value = 0ULL; // equal to prev, i.e. to value of toc_reserve
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    toc_key = cgleaf_off_end;
    toc_value = cglbuf_end = root_block_off; // limited by first block
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    // end of toc:
    toc_key = 0;
    toc_value = 0ULL;
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    detail::block_type root;
    root.set_block_size( bsz );
    root.set_flags( detail::block_type::root_node | detail::block_type::leaf_node );

    unsigned int root_address = append(root);
    assert(root_address == root_block_off );
  } else {
    file_.open(filename, (mode & std::ios_base::out) ? std::ios_base::in | std::ios_base::out : std::ios_base::in );
    uint8_t toc_key;
    uint64_t toc_value;

    file_.read( reinterpret_cast<char*>(&toc_key), sizeof(toc_key) );
    file_.read( reinterpret_cast<char*>(&toc_value), sizeof(toc_value) );

    if ( file_.fail() || (toc_key != magic) || (toc_value != magic_id) ) {
      return; // signal about fail: bad file format
    }

    off_type reserve = 0;

    for ( ; ; ) {
      file_.read( reinterpret_cast<char*>(&toc_key), sizeof(toc_key) );
      file_.read( reinterpret_cast<char*>(&toc_value), sizeof(toc_value) );
      if ( file_.fail() ) {
        return; // signal about fail: bad file format
      }

      switch ( toc_key ) {
        case cgleaf_off:
          cglbuf_beg = toc_value;
          break;
        case cgleaf_off_end:
          cglbuf_end = toc_value;
          break;
        case ver:
          format_ver = toc_value;
          break;
        case defbsz:
          bsz = toc_value;
          break;
        case rb_off:
          root_block_off = toc_value;
          break;
        case toc_reserve:
          reserve = toc_value;
          break;
        case 0:
          if ( toc_value == 0ULL ) {
            return; // ok, end of toc
          }
          break;
        default:
          break;
      }
    }
  }
}

void BTree::clear_cache()
{
  cache_.clear();
}

void BTree::close()
{
  // commits graph leafs not written to file here,
  // and may be lost; see/use BTree::flush.

  // dump all unwritten
  // fill/dump control structs
  file_.clear();
  file_.close();
}

BTree::off_type BTree::add_value( const char* data, std::streamsize size )
{
  return append_data( data, size );
}

std::string BTree::operator []( const key_type& key ) const throw(std::invalid_argument, std::runtime_error, std::bad_alloc, std::ios_base::failure )
{
  coordinate_type c = lookup( key );

  const detail::block_type& b = get( c );

  detail::block_type::const_iterator i = b.find( key );

  if ( i == b.end() ) {
    throw std::invalid_argument( "bad key" );
  }

  std::string s;

  s.reserve( i->second.size );
  file_.seekg( i->second.address );
  copy_n( istreambuf_iterator<char>(file_), i->second.size, back_inserter(s) );

  if ( file_.fail() ) {
    throw ios_base::failure( "BTree io operation fail" );
  }

  return s;
}

void BTree::insert( const key_type& key, const std::string& value )
{
  detail::block_coordinate addr;

  addr.address = add_value( value.data(), value.size() );
  addr.size = value.size();

  coordinate_type c = lookup( key );

  insert( c, key, addr );
}

} // namespace yard

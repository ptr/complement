// -*- C++ -*- Time-stamp: <2011-02-15 16:28:44 ptr>

/*
 *
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

// #include "yard.h"
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
#include <stack>
#include <list>
#include <limits>
#include <iterator>

namespace yard {

using namespace std;

void write_data( std::fstream& file, BTree::off_type address, const char* data, std::streamsize size )
{
  file.seekp( address );
  file.write( data, size );
}

BTree::off_type seek_to_end( std::fstream& file, std::streamsize size )
{
  const unsigned int alignment = 4096;

  file.seekp( 0, ios_base::end );
  BTree::off_type address = file.tellp();

  int over = address % alignment;
  BTree::off_type delta = over ? alignment - over : 0;
  file.seekp( delta, ios_base::end );

  return address + delta;
}

BTree::off_type append_data( std::fstream& file, const char* data, std::streamsize size )
{
  BTree::off_type address = seek_to_end( file, size );
  write_data( file, address, data, size );

  return address;
}

void get_data( std::fstream& file, BTree::off_type address, char* data, std::streamsize size )
{
  file.seekg( address );
  file.read( data, size );
}

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

bool BTree::is_open()
{ return file_.is_open(); }

bool BTree::good() const
{ return file_.good(); }

bool BTree::bad() const
{ return file_.bad(); }

BTree::off_type BTree::append(const detail::block_type& block)
{
  off_type address = seek_to_end(file_, bsz );
  block.pack(file_);
  return address;
}

void BTree::save(off_type offset, const detail::block_type& block)
{
    file_.seekp(offset, ios_base::beg);
    block.pack(file_);
}

void BTree::load(off_type offset, detail::block_type& block)
{
    file_.seekg(offset, ios_base::beg);
    block.unpack(file_);
}

detail::block_type& BTree::get_block(off_type offset)
{
  map<off_type, detail::block_type>::iterator it = cache_.find(offset);
    if (it == cache_.end())
    {
      detail::block_type& result = cache_[offset];
        result.set_block_size( bsz );
        load(offset, result);
        return result;
    }
    else
    {
        return it->second;
    }
}

BTree::key_type BTree::min_in_subtree( off_type block_address )
{
  detail::block_type& block = get_block(block_address);

    assert(block.begin() != block.end());
    if (block.is_leaf())
    {
        return block.begin()->first;
    }
    else
    {
        off_type address_of_new_block = block.begin()->second.address;
        return min_in_subtree(address_of_new_block);
    }
}

BTree::key_type BTree::max_in_subtree( off_type block_address )
{
  detail::block_type& block = get_block(block_address);

    assert(block.begin() != block.end());
    if (block.is_leaf())
    {
        return (--block.end())->first;
    }
    else
    {
        off_type address_of_new_block = (--block.end())->second.address;
        return max_in_subtree(address_of_new_block);
    }
}


BTree::off_type BTree::add_value( const char* data, streamsize size )
{
    return append_data(file_, data, size);
}

BTree::coordinate_type BTree::lookup(const coordinate_type& start, const key_type& key)
{
    if (start.empty())
        return lookup(key);

    coordinate_type result = start;
    pair<key_type, key_type>& segment = result.top().second;

    while ((segment.first > key) || (segment.second < key))
    {
        result.pop();

        assert(!result.empty());
        segment = result.top().second;
    }

    lookup_down(result, key);
    return result;
}

BTree::key_type prev_uuid(BTree::key_type key)
{
    assert(!((key.u.l[0] == 0) && (key.u.l[1] == 0)));

    if (key.u.l[1] == 0)
        --key.u.l[0];

    --key.u.l[1];
    return key;
}

void BTree::lookup_down(coordinate_type& path, const key_type& key)
{
    key_type max_uuid;
    max_uuid.u.l[0] = numeric_limits<uint64_t>::max();
    max_uuid.u.l[1] = numeric_limits<uint64_t>::max();

    while (true)
    {
      detail::block_type& new_block = get_block(path.top().first);

        if (new_block.is_leaf())
            return;

        std::pair<key_type, key_type> segment;

        detail::block_type::const_iterator next_block = new_block.route(key);
        segment.first = next_block->first;

        detail::block_type::const_iterator next_next_block = next_block;
        advance(next_next_block, 1);

        segment.second = next_next_block == new_block.end() ? max_uuid : prev_uuid(next_next_block->first);

        path.push(make_pair(next_block->second.address, segment));
    }
}

BTree::coordinate_type BTree::lookup(const key_type& key)
{
    key_type max_uuid, min_uuid;

    max_uuid.u.l[0] = numeric_limits<uint64_t>::max();
    max_uuid.u.l[1] = numeric_limits<uint64_t>::max();

    min_uuid.u.l[0] = 0;
    min_uuid.u.l[1] = 0;

    coordinate_type path;
    path.push(make_pair( root_block_off, make_pair(min_uuid, max_uuid)));

    lookup_down(path, key);

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

  detail::block_type& block = get_block(desc.first);
  bool result = block.insert(key, coord);
  assert(result);

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
    detail::block_type& parent_block = get_block(path.top().first);

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
    key_type zero_key;
    zero_key.u.l[0] = zero_key.u.l[1] = 0;

    detail::block_coordinate zero_coord;
    zero_coord.address = address_of_block;
    zero_coord.size = bsz;

    bool result = new_root.insert(zero_key, zero_coord);
    assert(result);
  }

  result = new_root.insert(new_key, new_coord);
  assert(result);
  cache_[ root_block_off ] = new_root;

  save( root_block_off, new_root);

  return path;
}

const detail::block_type& BTree::get(const coordinate_type& coordinate)
{
  return get_block(coordinate.top().first);
}

void BTree::open( const char* filename, std::ios_base::openmode mode, std::streamsize block_size )
{
  if ( (mode & std::ios_base::trunc) ) {
    file_.open(filename, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

    format_ver = 0;
    bsz = block_size;
    root_block_off = 4096;

    uint8_t  toc_key   = magic;
    uint64_t toc_value = 0x81a5c36900000000LL;
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

    toc_key = cgleaf_off;
    toc_value = 0ULL;
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
    toc_value = (std::max)(static_cast<std::streambuf::off_type>(static_cast<std::streambuf::off_type>(file_.tellp()) + (sizeof(toc_key) + sizeof(toc_value)) * 2), static_cast<std::streambuf::off_type>(128) );
    file_.write( reinterpret_cast<const char*>(&toc_key), sizeof(toc_key) );
    file_.write( reinterpret_cast<const char*>(&toc_value), sizeof(toc_value) );

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
    file_.open(filename, std::ios_base::in | std::ios_base::out);
    uint8_t toc_key;
    uint64_t toc_value;

    file_.read( reinterpret_cast<char*>(&toc_key), sizeof(toc_key) );
    file_.read( reinterpret_cast<char*>(&toc_value), sizeof(toc_value) );

    if ( file_.fail() || (toc_key != magic) || (toc_value != 0x81a5c36900000000LL) ) {
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
            if ( reserve != 0 ) {
              file_.seekp( reserve, ios_base::beg );
              file_.seekg( reserve, ios_base::beg );
            }
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
  // dump all unwritten
  // fill/dump control structs
  file_.close();
}

void BTree::flush()
{
  // write all info to file,
  // ...
  // and flush file
  file_.flush();
}

} // namespace yard

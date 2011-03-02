// -*- C++ -*- Time-stamp: <2011-03-02 18:54:52 ptr>

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

#include <istream>
#include <ostream>

#include <sstream>
#include <iomanip>
#include <functional>

#include <algorithm>
#include <functional>
#include <cassert>

#if !defined(STLPORT) && defined(__GNUC__)
#  include <ext/functional>
#endif

#include <iterator>

namespace yard {

namespace detail {

using namespace std;

typedef uuid_packer_exp index_key_packer;
typedef varint_packer index_address_packer;

block_type::block_type()
{
  flags_ = 0;
  size_of_packed_ = 3 * packer_traits<uint32_t, std_packer>::max_size();
}

void block_type::set_block_size( streamsize block_size )
{
  block_size_ = block_size;
}

std::streamsize block_type::get_block_size() const
{
  return block_size_;
}

bool block_type::is_root() const
{
  return ((flags_ & root_node) == root_node);
}

bool block_type::is_leaf() const
{
  return ((flags_ & leaf_node) == leaf_node);
}

bool block_type::is_overfilled() const
{
  return (size_of_packed_ > get_block_size());
}

std::streamsize block_type::size_of_packed_entry( const_iterator it )
{
  return is_leaf() ?
    packer_traits<key_type, std_packer>::size(it->first) +
    packer_traits<uint32_t, std_packer>::size(it->second.address) +
    packer_traits<uint32_t, std_packer>::size(it->second.size)
    :
    packer_traits<key_type, index_key_packer>::size(it->first) +
    packer_traits<uint32_t, index_address_packer>::size(it->second.address);
}

int block_type::erase( const key_type& key )
{
  iterator it = body_.find(key);

  if ( it == body_.end() ) {
    return 0;
  }

  size_of_packed_ -= size_of_packed_entry( it );
  body_.erase( it );

  return 1;
}

bool block_type::insert( const key_type& key, const block_coordinate& coordinate )
{
  pair<body_type::iterator, bool> result = body_.insert(body_type::value_type(key, coordinate));

  if ( result.second ) {
    size_of_packed_ += size_of_packed_entry( result.first );
  }

  return result.second;
}

xmt::uuid_type block_type::min() const
{
  assert(!body_.empty());
  return body_.begin()->first;
}

xmt::uuid_type block_type::max() const
{
  assert(!body_.empty());
  return (--body_.end())->first;
}

void block_type::calculate_size()
{
  size_of_packed_ = 3 * packer_traits<uint32_t, std_packer>::max_size();

  for ( body_type::iterator it = body_.begin(); it != body_.end(); ++it ) {
    if ( is_leaf() ) {
      // suspected place:
      // calculated size may not correspont to really written bytes
      size_of_packed_ += packer_traits<key_type, std_packer>::size(it->first);
      size_of_packed_ += packer_traits<uint32_t, std_packer>::size(it->second.address);
      size_of_packed_ += packer_traits<uint32_t, std_packer>::size(it->second.size);
    } else {
      // suspected place:
      // calculated size may not correspont to really written bytes
      size_of_packed_ += packer_traits<key_type, index_key_packer>::size(it->first);
      size_of_packed_ += packer_traits<uint32_t, index_address_packer>::size(it->second.address);
    }
  }
}

void block_type::divide(block_type& other)
{
  assert(is_overfilled());

  if ( is_root() ) {
    flags_ = flags_ ^ root_node;
  }
  other.flags_ = flags_;
  other.block_size_ = block_size_;

  std::streamsize size_of_new_body = 0;
  iterator it = body_.begin();
  while ( (it != body_.end()) && (size_of_new_body < size_of_packed_ / 2) ) {
    size_of_new_body += size_of_packed_entry(it);
    ++it;
  }
  assert(it != body_.end());

  const iterator middle = it;

  other.body_.insert(middle, body_.end());
  body_.erase(middle, body_.end());

  if ( !other.is_leaf() ) {
    //add erasing of the first key in the other
  }

  other.size_of_packed_ = size_of_packed_ - size_of_new_body;
  size_of_packed_ = 3 * packer_traits<uint32_t, std_packer>::max_size() + size_of_new_body;

  calculate_size();
  other.calculate_size();

  assert(size_of_packed_ <= block_size_);
  assert(other.size_of_packed_ <= other.block_size_);

  assert(begin() != end());
  assert(other.begin() != other.end());
}

block_type::const_iterator block_type::begin() const
{
  return body_.begin();
}

block_type::const_iterator block_type::end() const
{
  return body_.end();
}

block_type::const_iterator block_type::find(const key_type& key) const
{
  return body_.find(key);
}

block_type::const_iterator block_type::route(const key_type& key) const
{
  assert(!is_leaf());
  assert(!body_.empty());

  if (body_.empty()) {
    return body_.end();
  }

  for ( const_iterator it = ++body_.begin(); it != body_.end(); ++it ) {
    if ( it->first > key ) {
      return --it;
    }
  }

  return --body_.end();
}

void block_type::pack( std::ostream& s ) const
{
  assert(get_block_size() > 0);

  std::ostream::streampos begin_pos = s.tellp();

  std_packer::pack<uint32_t>(s, 0);
  std_packer::pack(s, static_cast<uint32_t>(flags_) );
  std_packer::pack(s, static_cast<uint32_t>(body_.size()) );

  for ( const_iterator it = body_.begin(); it != body_.end(); ++it ) {
    if ( is_leaf() ) {
      std_packer::pack( s, it->first);
      std_packer::pack( s, static_cast<uint32_t>(it->second.address) );
      std_packer::pack( s, static_cast<uint32_t>(it->second.size) );
    } else {
      index_key_packer::pack( s, it->first );
      index_address_packer::pack( s, static_cast<uint32_t>(it->second.address) );
    }
  }
  
  std::ostream::streampos end_pos = s.tellp();
  assert(end_pos - begin_pos <= get_block_size());

  streamsize size = end_pos - begin_pos;
  if ( size != get_block_size() ) {
    vector<char> buffer( get_block_size() - size );
    s.write( &buffer[0], buffer.size() );
  }
}

void block_type::unpack( std::istream& s )
{
  uint32_t version;
  std_packer::unpack(s, version);
  assert( version == 0 );

  uint32_t tmp;
  std_packer::unpack(s, tmp );
  flags_ = static_cast<unsigned>(tmp);
  std_packer::unpack(s, tmp );
  body_type::size_type size = static_cast<body_type::size_type>(tmp);

  key_type key;
  block_coordinate coordinate;

  assert(body_.empty());

  for ( body_type::size_type i = 0; i < size; ++i ) {
    if ( is_leaf() ) {
      std_packer::unpack( s, key );
      std_packer::unpack( s, tmp );
      coordinate.address = static_cast<block_coordinate::off_type>(tmp);
      std_packer::unpack( s, tmp );
      coordinate.size = static_cast<size_t>(tmp);
    } else {
      index_key_packer::unpack( s, key );
      index_address_packer::unpack( s, tmp );
      coordinate.address = static_cast<block_coordinate::off_type>(tmp);
      coordinate.size = get_block_size();
    }

    insert( key, coordinate );
  }
}

void block_type::set_flags( unsigned int flags )
{
  flags_ = flags;
}

} // namespace detail

} // namespace yard

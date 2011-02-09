// -*- C++ -*- Time-stamp: <2011-02-09 20:20:50 ptr>

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

BTree::block_type::block_type()
{
  flags_ = 0;
  size_of_packed_ = 3 * packer_traits<uint32_t, std_packer>::max_size();
}

void BTree::block_type::set_block_size( streamsize block_size )
{
  block_size_ = block_size;
}

std::streamsize BTree::block_type::get_block_size() const
{
  return block_size_;
}

bool BTree::block_type::is_root() const
{
  return ((flags_ & root_node) == root_node);
}

bool BTree::block_type::is_leaf() const
{
  return ((flags_ & leaf_node) == leaf_node);
}

bool BTree::block_type::is_overfilled() const
{
  return (size_of_packed_ > get_block_size());
}

int BTree::block_type::erase(const key_type& key)
{
    iterator it = body_.find(key);
    if (it == body_.end())
        return 0;
    else
    {
        if (is_leaf())
        {
          // suspected place:
          // calculated size may not correspont to really written bytes
            size_of_packed_ -=
                packer_traits<key_type, std_packer>::size(it->first) +
                packer_traits<uint32_t, std_packer>::max_size() +
                packer_traits<uint32_t, std_packer>::max_size();
        }
        else
        {
          // suspected place:
          // calculated size may not correspont to really written bytes
           size_of_packed_ -=
                packer_traits<key_type, uuid_packer_exp>::size(it->first) +
                packer_traits<uint32_t, varint_packer>::size(it->second.address);
        }
        body_.erase(it);
        return 1;
    }
}

bool BTree::block_type::insert(const key_type& key, const block_coordinate& coordinate)
{
    pair<body_type::iterator, bool> result = body_.insert(body_type::value_type(key, coordinate));

    if (result.second)
    {
        if (is_leaf())
        {
          // suspected place:
          // calculated size may not correspont to really written bytes
            size_of_packed_ +=
                packer_traits<key_type, std_packer>::size(key) +
                packer_traits<uint32_t, std_packer>::max_size() +
                packer_traits<uint32_t, std_packer>::max_size();
        }
        else
        {
          // suspected place:
          // calculated size may not correspont to really written bytes
           size_of_packed_ +=
                packer_traits<key_type, uuid_packer_exp>::size(key) +
                packer_traits<uint32_t, varint_packer>::size(coordinate.address);
        }
    }

    return result.second;
}

xmt::uuid_type BTree::block_type::min() const
{
    assert(!body_.empty());
    return body_.begin()->first;
}

xmt::uuid_type BTree::block_type::max() const
{
    assert(!body_.empty());
    return (--body_.end())->first;
}

void BTree::block_type::calculate_size()
{
    size_of_packed_ = 3 * packer_traits<uint32_t, std_packer>::max_size();

    for (body_type::iterator it = body_.begin();
         it != body_.end(); ++it)
    {
        if (is_leaf())
        {
          // suspected place:
          // calculated size may not correspont to really written bytes
            size_of_packed_ += packer_traits<key_type, std_packer>::size(it->first);
            size_of_packed_ += packer_traits<uint32_t, std_packer>::max_size();
            size_of_packed_ += packer_traits<uint32_t, std_packer>::max_size();
        }
        else
        {
          // suspected place:
          // calculated size may not correspont to really written bytes
            size_of_packed_ += packer_traits<key_type, uuid_packer_exp>::size(it->first);
            size_of_packed_ += packer_traits<uint32_t, varint_packer>::size(it->second.address);
        }
    }
}

void BTree::block_type::divide(block_type& other)
{
    assert(is_overfilled());

    if (is_root())
    {
        flags_ = flags_ ^ root_node;
    }
    other.flags_ = flags_;
    other.block_size_ = block_size_;

    body_type::iterator middle = body_.begin();
    std::advance(middle, body_.size() / 2);

    other.body_.insert(middle, body_.end());
    body_.erase(middle, body_.end());

    if (!other.is_leaf())
    {
        //add erasing of the first key in the other
    }

    other.calculate_size();
    calculate_size();

    assert(begin() != end());
    assert(other.begin() != other.end());
}

BTree::block_type::const_iterator BTree::block_type::begin() const
{
  return body_.begin();
}

BTree::block_type::const_iterator BTree::block_type::end() const
{
  return body_.end();
}

BTree::block_type::const_iterator BTree::block_type::find(const key_type& key) const
{
  return body_.find(key);
}

BTree::block_type::const_iterator BTree::block_type::route(const key_type& key) const
{
    assert(!is_leaf());
    assert(!body_.empty());

    if (body_.empty())
        return body_.end();

    for (const_iterator it = ++body_.begin();
         it != body_.end();
         ++it)
    {
        if (it->first > key)
            return --it;
    }

    return --body_.end();
}

void BTree::block_type::pack( std::ostream& s ) const
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
      uuid_packer_exp::pack( s, it->first );
      varint_packer::pack( s, static_cast<uint32_t>(it->second.address) );
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

void BTree::block_type::unpack( std::istream& s )
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
      coordinate.address = static_cast<off_type>(tmp);
      std_packer::unpack( s, tmp );
      coordinate.size = static_cast<size_t>(tmp);
    } else {
      uuid_packer_exp::unpack( s, key );
      varint_packer::unpack( s, tmp );
      coordinate.address = static_cast<off_type>(tmp);
      coordinate.size = get_block_size();
    }

    insert( key, coordinate );
  }
}

void BTree::block_type::set_flags( unsigned int flags )
{
  flags_ = flags;
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

BTree::off_type BTree::append(const block_type& block)
{
  off_type address = seek_to_end(file_, bsz );
  block.pack(file_);
  return address;
}

void BTree::save(off_type offset, const block_type& block)
{
    file_.seekp(offset, ios_base::beg);
    block.pack(file_);
}

void BTree::load(off_type offset, block_type& block)
{
    file_.seekg(offset, ios_base::beg);
    block.unpack(file_);
}

BTree::block_type& BTree::get_block(off_type offset)
{
    map<off_type, block_type>::iterator it = cache_.find(offset);
    if (it == cache_.end())
    {
        block_type& result = cache_[offset];
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
    block_type& block = get_block(block_address);

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
    block_type& block = get_block(block_address);

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
        block_type& new_block = get_block(path.top().first);

        if (new_block.is_leaf())
            return;

        std::pair<key_type, key_type> segment;

        block_type::const_iterator next_block = new_block.route(key);
        segment.first = next_block->first;

        block_type::const_iterator next_next_block = next_block;
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

BTree::coordinate_type BTree::insert(coordinate_type path, const key_type& key, const block_coordinate& coord)
{
    const block_desc& desc = path.top();

    block_type& block = get_block(desc.first);
    bool result = block.insert(key, coord);
    assert(result);

    if (block.is_overfilled())
    {
        block_type new_block;
        block.divide(new_block);

        pair<xmt::uuid_type, xmt::uuid_type> delimiter;
        if (block.is_leaf())
        {
            delimiter.first = (--block.end())->first;
            delimiter.second = new_block.begin()->first;
        }
        else
        {
            delimiter.first = max_in_subtree((--block.end())->second.address);
            delimiter.second = min_in_subtree(new_block.begin()->second.address);
        }

        off_type address_of_block = append(block);
        off_type address_of_new_block = append(new_block);

        cache_[address_of_block] = block;
        cache_[address_of_new_block] = new_block;

        cache_.erase(desc.first);

        key_type new_key = get_shortest_key(delimiter.first, delimiter.second);

        block_coordinate new_coord;
        new_coord.address = address_of_new_block;
        new_coord.size = bsz;

        key_type key_to_update = desc.second.first;

        path.pop();

        if (path.empty())
        {
            block_type new_root;
            new_root.set_block_size( bsz );
            new_root.set_flags(block_type::root_node);

            {
                key_type zero_key;
                zero_key.u.l[0] = zero_key.u.l[1] = 0;

                block_coordinate zero_coord;
                zero_coord.address = address_of_block;
                zero_coord.size = bsz;

                bool result = new_root.insert(zero_key, zero_coord);
                assert(result);
            }

            bool result = new_root.insert(new_key, new_coord);
            assert(result);
            cache_[ root_block_off ] = new_root;

            save( root_block_off, new_root);
        }
        else
        {
            block_type& parent_block = get_block(path.top().first);

            int count = parent_block.erase(key_to_update);
            assert(count == 1);

            block_coordinate coord;
            coord.address = address_of_block;
            coord.size = bsz;

            bool result = parent_block.insert(key_to_update, coord);
            assert(result);

            return insert(path, new_key, new_coord);
        }
    }
    else
    {
        save(path.top().first, block);
    }

    return path;
}

const BTree::block_type& BTree::get(const coordinate_type& coordinate)
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

    block_type root;
    root.set_block_size( bsz );
    root.set_flags(block_type::root_node | block_type::leaf_node);

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
    // Try to upload from disk
    throw std::invalid_argument( "invalid revision" );
  }

  return i->second.content;
}

void revision::get_manifest( manifest_type& m, const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    // Try to upload from disk
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
    // Try to upload from disk
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

void revision::get_commit( commit_node& c, const revision_id_type& rid ) throw( std::invalid_argument )
{
  revisions_container_type::iterator i = r.find( rid );

  if ( i == r.end() ) {
    // Try to upload from disk
    throw std::invalid_argument( "invalid revision" );
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
}

yard::yard()
{
  manifest_type m;
  
  manifest_id_type mid = r.push( m ); // ToDo: clear mod flag in r

  cached_manifest[mid]; // = m;
  c[xmt::nil_uuid].mid = mid; // root
  c[xmt::nil_uuid].delta = 0; // root
  c[xmt::nil_uuid].dref = -1;
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

} // namespace yard

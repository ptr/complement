// -*- C++ -*- Time-stamp: <2011-03-04 18:51:13 ptr>

/*
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "yard_perf.h"

#include <yard/yard.h>

#include <fstream>
#include <vector>
#include <unistd.h>

using namespace std;

yard_perf::yard_perf()
{
}

yard_perf::~yard_perf()
{
}

int EXAM_IMPL(yard_perf::packing)
{
  using namespace yard;
  using yard::detail::block_type;
  using yard::detail::block_coordinate;

  std::ofstream file( "/tmp/pack_unpack", ios_base::out | ios_base::trunc );

  block_type block;
  block.set_block_size(4096);
  block.set_flags( block_type::leaf_node );

  BTree::key_type key;
  block_coordinate coord;

  for ( int i = 0; i < 50; ++i ) {
    key.u.l[0] = 3*i;
    key.u.l[1] = 0;

    coord.address = i;
    coord.size = 2*i;

    block.insert(key, coord);
  }

  const int count = 10000;
  for ( int i = 0; i < count; ++i ) {
    block.pack(file);
  }

  // file.close();

  // unlink( "/tmp/pack_unpack" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::unpacking)
{
  using namespace yard;
  using yard::detail::block_type;
  using yard::detail::block_coordinate;

  std::ifstream file( "/tmp/pack_unpack", ios_base::in );

  block_type block;
  block.set_block_size(4096);

  const int count = 10000;
  for ( int i = 0; i < count; ++i ) {
    file.seekg(i* 4096, ios_base::beg);
    block.unpack(file);
  }

  // file.close();

  // unlink( "/tmp/pack_unpack" );

  return EXAM_RESULT;
}

void fill_consecutive( yard::BTree& tree, int count )
{
  using namespace yard;
  using yard::detail::block_type;
  using yard::detail::block_coordinate;

  BTree::key_type key;
  block_coordinate coord;

  for ( int i = 0; i < count; ++i ) {
    key.u.l[0] = 3*i;
    key.u.l[1] = 0;

    coord.address = i;
    coord.size = 2*i;

    BTree::coordinate_type coordinate = tree.lookup(key);

    tree.insert(coordinate, key, coord);
  }
}

int EXAM_IMPL(yard_perf::consecutive_insert)
{
  using namespace yard;

  {
    BTree tree( "/tmp/btree_consecutive", std::ios_base::in | std::ios_base::out | std::ios_base::trunc, 4096 );

    fill_consecutive(tree, 100000);
  }

  // unlink( "/tmp/btree_consecutive" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::consecutive_insert_big)
{
  using namespace yard;

  {
    BTree tree( "/tmp/btree_consecutive", std::ios_base::in | std::ios_base::out | std::ios_base::trunc, 4096 );

    fill_consecutive(tree, 1000000);
  }

  // unlink( "/tmp/btree_consecutive" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::random_insert_big)
{
  using namespace yard;
  using yard::detail::block_coordinate;
 
  {  
    BTree tree( "/tmp/btree_random", std::ios_base::in | std::ios_base::out | std::ios_base::trunc, 4096 );

    const int count = 1000000;

    BTree::key_type key;
    block_coordinate coord;

    for ( int i = 0; i < count; ++i ) {
      key.u.l[0] = rand();
      key.u.l[1] = 0;

      coord.address = rand();
      coord.size = rand();

      BTree::coordinate_type coordinate = tree.lookup(key);

      tree.insert(coordinate, key, coord);
    }
  }

  // unlink( "/tmp/btree_random" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::consecutive_insert_with_data)
{
  using namespace yard;
  using yard::detail::block_coordinate;

  const int data_size = 4096;
  char data[data_size];
  for ( int i = 0; i < data_size; ++i ) {
    data[i] = (char)(rand() % 256);
  }

  {
    BTree tree( "/tmp/btree_consecutive_with_data", std::ios_base::in | std::ios_base::out | std::ios_base::trunc, 4096 );

    const int count = 40000;
    BTree::key_type key;
    block_coordinate coord;

    for ( int i = 0; i < count; ++i ) {
      key.u.l[0] = 3*i;
      key.u.l[1] = 0;

      BTree::coordinate_type coordinate = tree.lookup(key);

      coord.address = tree.add_value(data, data_size);
      coord.size = data_size;
      tree.insert(coordinate, key, coord);
    }
  }

  // unlink( "/tmp/btree_consecutive_with_data" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::random_insert_with_data)
{
  using namespace yard;
  using yard::detail::block_coordinate;

  const int data_size = 4096;
  char data[data_size];
  for ( int i = 0; i < data_size; ++i ) {
    data[i] = (char)(rand() % 256);
  }

  {
    BTree tree( "/tmp/btree_random_with_data", std::ios_base::in | std::ios_base::out | std::ios_base::trunc, 4096 );

    const int count = 40000;
    BTree::key_type key;
    block_coordinate coord;

    for ( int i = 0; i < count; ++i ) {
      key.u.l[0] = rand();
      key.u.l[1] = 0;

      BTree::coordinate_type coordinate = tree.lookup(key);

      coord.address = tree.add_value(data, data_size);
      coord.size = data_size;
      tree.insert(coordinate, key, coord);
    }
  }

  // unlink( "/tmp/btree_random_with_data" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::multiple_files)
{
  using namespace yard;
  using yard::detail::block_coordinate;

  const int data_size = 4096;
  char data[data_size];
  for (int i = 0; i < data_size; ++i) {
    data[i] = (char)(rand() % 256);
  }

  const int file_count = 8;

  {
    BTree trees[file_count];

    for ( int i = 0; i < file_count; ++i ) {
      stringstream ss("/tmp/btree_", ios_base::ate | ios_base::out);
      ss << i;
      trees[i].open(ss.str().c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc, 4096);
    }

    const int count = 5000;
    BTree::key_type key;
    block_coordinate coord;

    for ( int k = 0; k < count; ++k ) {
      for ( int i = 0; i < file_count; ++i ) {
        key.u.l[0] = rand();
        key.u.l[1] = 0;

        BTree::coordinate_type coordinate = trees[i].lookup(key);

        coord.address = trees[i].add_value(data, data_size);
        coord.size = data_size;
        trees[i].insert(coordinate, key, coord);
      }
    }
  }

  for ( int i = 0; i < file_count; ++i ) {
    stringstream ss("/tmp/btree_", ios_base::ate | ios_base::out);
    ss << i;
    // unlink( ss.str().c_str() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::random_lookup)
{
  using namespace yard;

  {
    BTree tree( "/tmp/btree_consecutive" );

    const int count = 10000;
    BTree::key_type key;

    for ( int i = 0; i < count; ++i ) {
      key.u.l[0] = 3*(rand() % 40000);
      key.u.l[1] = 0;

      BTree::coordinate_type coordinate = tree.lookup(key);
    }
  }

  // unlink( "/tmp/btree_consecutive" ); ?

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::put_revisions)
{
  yard::revision revs;
  try {
    const int countOfBlocks = 1000;
    const size_t blockSize = 2*1000;

    const size_t sz = (blockSize / sizeof(xmt::uuid_type)) * sizeof(xmt::uuid_type);
    string random_content( sz, 0 );
    for ( int i = 0; i < countOfBlocks; ++i) {
      for (int j = 0; j < sz; j+= sizeof(xmt::uuid_type)) {
        xmt::uuid_type b = xmt::uid();
        uninitialized_copy( b.u.b, b.u.b + sizeof(xmt::uuid_type), const_cast<char*>(random_content.data()) + i );
      }
      revs.push(random_content);
    }
  }
  catch( const std::invalid_argument& err ) {
    EXAM_ERROR(err.what());
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR(err.what());
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::mess)
{
  yard::yard db;

  try {
    yard::commit_id_type cid = xmt::nil_uuid;
    yard::commit_id_type cid2 = xmt::uid();
    string base_name( "/mess/" );
    const size_t sz = (2*1024 / sizeof(xmt::uuid_type)) * sizeof(xmt::uuid_type);
    string random_content( sz, 0 );

    for ( int count = 0; count < 1000; ++count ) {
      db.open_commit_delta( cid, cid2 );

      for ( int k = 0; k < 3; ++k ) {
        for ( int i = 0; i < sz; i += sizeof(xmt::uuid_type) ) {
          xmt::uuid_type b = xmt::uid();
          uninitialized_copy( b.u.b, b.u.b + sizeof(xmt::uuid_type), const_cast<char*>(random_content.data()) + i );
        }
        db.add( cid2, base_name + xmt::uid_str(), random_content );
      }

      db.close_commit_delta( cid2 );
      cid = cid2;
      cid2 = xmt::uid();
    }
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::mess_insert)
{
  const char fn[] = "./btree";

  string content( 1024, 'A' );
  string name( "/AAAAAA" );

  try {
    yard::commit_id_type cid0;
    yard::commit_id_type cid1;

    for ( int i = 0; i < 1000; ++i ) {
      yard::yard db( fn );

      if ( !db.is_open() /* || !db.good() */ ) {
        db.clear();
        db.open( fn, ios_base::trunc );
      }

      content[512] = 'A' + (i % 0x40);
      content[513] = 'A' + (i >> 6) % 0x40;
      content[514] = 'A' + (i >> 12) % 0x40;

      name[1] = 'A' + (i % 0x40);
      name[2] = 'A' + (i >> 6) % 0x40;
      name[3] = 'A' + (i >> 12) % 0x40;

      if ( i == 0 ) {
        cid0 = xmt::nil_uuid;
      } else {
        std::list<yard::commit_id_type> h;

        db.heads( back_inserter(h) );
        if ( !h.empty() ) {
          cid0 = h.front();
        } else {
          EXAM_ERROR( "no leafs in commits graph?" );
        }
      }

      cid1 = xmt::uid();

      db.open_commit_delta( cid0, cid1 );

      db.add( cid1, name, content );

      db.close_commit_delta( cid1 );

      EXAM_CHECK( db.is_open() );
      EXAM_CHECK( db.good() );
    }
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( fn );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::mess_insert_single_commit)
{
  const char fn[] = "./btree";

  string content( 1024, 'A' );
  string name( "/AAAAAA" );

  try {
    yard::commit_id_type cid0 = xmt::nil_uuid;
    yard::commit_id_type cid1 = xmt::uid();

    yard::yard db( fn, ios_base::trunc );

    db.open_commit_delta( cid0, cid1 );

    for ( int i = 0; i < 1000; ++i ) {
      content[512] = 'A' + (i % 0x40);
      content[513] = 'A' + (i >> 6) % 0x40;
      content[514] = 'A' + (i >> 12) % 0x40;

      name[1] = 'A' + (i % 0x40);
      name[2] = 'A' + (i >> 6) % 0x40;
      name[3] = 'A' + (i >> 12) % 0x40;

      db.add( cid1, name, content );

      EXAM_CHECK( db.is_open() );
      EXAM_CHECK( db.good() );
    }

    db.close_commit_delta( cid1 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( fn );

  return EXAM_RESULT;
}

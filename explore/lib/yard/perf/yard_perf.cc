// -*- C++ -*- Time-stamp: <2011-02-02 18:24:14 ptr>

/*
 * Copyright (c) 2010
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
    std::ofstream file("/tmp/pack_unpack", ios_base::out | ios_base::trunc);

    BTree::block_type block;
    block.set_block_size(4096);
    block.set_flags(BTree::block_type::leaf_node);

    BTree::key_type key;
    BTree::block_coordinate coord;

    for (int i = 0; i < 50; ++i)
    {
        key.u.l[0] = 3*i;
        key.u.l[1] = 0;

        coord.address = i;
        coord.size = 2*i;

        block.insert(key, coord);
    }

    const int count = 10000;
    for (int i = 0; i < count; ++i)
        block.pack(file);

    return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::unpacking)
{
    using namespace yard;
    std::ifstream file("/tmp/pack_unpack", ios_base::in);

    BTree::block_type block;
    block.set_block_size(4096);

    const int count = 10000;
    for (int i = 0; i < count; ++i)
    {
        file.seekg(i* 4096, ios_base::beg);
        block.unpack(file);
    }

    return EXAM_RESULT;
}

void fill_consecutive(yard::BTree& tree, int count)
{
    using namespace yard;
    BTree::key_type key;
    BTree::block_coordinate coord;

    for (int i = 0; i < count; ++i)
    {
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

    BTree tree;
    tree.init_empty("/tmp/btree_consecutive", 4096);

    fill_consecutive(tree, 100000);
    return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::consecutive_insert_big)
{
    using namespace yard;

    BTree tree;
    tree.init_empty("/tmp/btree_consecutive", 4096);

    fill_consecutive(tree, 1000000);
    return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::random_insert_big)
{
    using namespace yard;
    BTree tree;
    tree.init_empty("/tmp/btree_random", 4096);

    const int count = 1000000;

    BTree::key_type key;
    BTree::block_coordinate coord;

    for (int i = 0; i < count; ++i)
    {
        key.u.l[0] = rand();
        key.u.l[1] = 0;

        coord.address = rand();
        coord.size = rand();

        BTree::coordinate_type coordinate = tree.lookup(key);

        tree.insert(coordinate, key, coord);
    }

    return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::consecutive_insert_with_data)
{
    using namespace yard;

    const int data_size = 4096;
    char data[data_size];
    for (int i = 0; i < data_size; ++i)
        data[i] = (char)(rand() % 256);

    BTree tree;
    tree.init_empty("/tmp/btree_consecutive_with_data", 4096);

    const int count = 40000;
    BTree::key_type key;
    BTree::block_coordinate coord;

    for (int i = 0; i < count; ++i)
    {
        key.u.l[0] = 3*i;
        key.u.l[1] = 0;

        BTree::coordinate_type coordinate = tree.lookup(key);

        coord.address = tree.add_value(data, data_size);
        coord.size = data_size;
        tree.insert(coordinate, key, coord);
    }
    return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::random_insert_with_data)
{
    using namespace yard;

    const int data_size = 4096;
    char data[data_size];
    for (int i = 0; i < data_size; ++i)
        data[i] = (char)(rand() % 256);

    BTree tree;
    tree.init_empty("/tmp/btree_random_with_data", 4096);

    const int count = 40000;
    BTree::key_type key;
    BTree::block_coordinate coord;

    for (int i = 0; i < count; ++i)
    {
        key.u.l[0] = rand();
        key.u.l[1] = 0;

        BTree::coordinate_type coordinate = tree.lookup(key);

        coord.address = tree.add_value(data, data_size);
        coord.size = data_size;
        tree.insert(coordinate, key, coord);
    }
    return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::multiple_files)
{
    using namespace yard;

    const int data_size = 4096;
    char data[data_size];
    for (int i = 0; i < data_size; ++i)
        data[i] = (char)(rand() % 256);

    const int file_count = 8;
    BTree trees[file_count];

    for (int i = 0; i < file_count; ++i)
    {
        stringstream ss("/tmp/btree_", ios_base::ate | ios_base::out);
        ss << i;
        trees[i].init_empty(ss.str().c_str(), 4096);
    }

    const int count = 5000;
    BTree::key_type key;
    BTree::block_coordinate coord;

    for (int k = 0; k < count; ++k)
    {
        for (int i = 0; i < file_count; ++i)
        {
            key.u.l[0] = rand();
            key.u.l[1] = 0;

            BTree::coordinate_type coordinate = trees[i].lookup(key);

            coord.address = trees[i].add_value(data, data_size);
            coord.size = data_size;
            trees[i].insert(coordinate, key, coord);
        }
    }
    return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::random_lookup)
{
    using namespace yard;

    BTree tree;
    tree.init_existed("/tmp/btree_consecutive");

    const int count = 10000;
    BTree::key_type key;

    for (int i = 0; i < count; ++i)
    {
        key.u.l[0] = 3*(rand() % 40000);
        key.u.l[1] = 0;

        BTree::coordinate_type coordinate = tree.lookup(key);
    }
    return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::put)
{
  const int nn = 1024;

  try {
    yard::underground db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      gen = xmt::uid();
      db.put_revision( &gen, sizeof(xmt::uuid_type) );
    }
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( "/tmp/yard" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::put_get)
{
  const int nn = 1024;

  vector<yard::id_type> data_key;

  data_key.reserve( nn );

  try {
    yard::underground db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      gen = xmt::uid();
      data_key.push_back( db.put_revision( &gen, sizeof(xmt::uuid_type) ) );
    }

    std::string tmp;

    for ( int i = 0; i < nn; ++i ) {
      tmp = db.get( data_key[i] );

      // cerr << i << ' ' << std::string(*reinterpret_cast<const xmt::uuid_type*>(tmp.data())) << ' ' << std::string(data_put[i]) << endl;
      // EXAM_CHECK( db.get( data_key[i] ) == std::string( reinterpret_cast<char*>(&data_put[i].u.b[0]), sizeof(xmt::uuid_type) ) );
    }
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( "/tmp/yard" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::put_more)
{
  const int nn = 10240;

  try {
    yard::underground db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      gen = xmt::uid();
      db.put_revision( &gen, sizeof(xmt::uuid_type) );
    }
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( "/tmp/yard" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::put_more_more)
{
  const int nn = 102400;

  try {
    yard::underground db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      gen = xmt::uid();
      db.put_revision( &gen, sizeof(xmt::uuid_type) );
    }
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( "/tmp/yard" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::put_object)
{
  const int nn = 1024;

  try {
    yard::underground db( "/tmp/yard" );

    xmt::uuid_type gen;
    xmt::uuid_type id;

    for ( int i = 0; i < nn; ++i ) {
      gen = xmt::uid();
      id = xmt::uid();
      db.put_object( id, &gen, sizeof(xmt::uuid_type) );
    }
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( "/tmp/yard" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::put_object_r2)
{
  const int nn = 1024;

  try {
    yard::underground db( "/tmp/yard" );

    xmt::uuid_type gen;
    xmt::uuid_type id;

    vector<yard::id_type> data_key;

    data_key.reserve( nn );

    for ( int i = 0; i < nn; ++i ) {
      gen = xmt::uid();
      id = xmt::uid();
      data_key.push_back( id );
      db.put_object( id, &gen, sizeof(xmt::uuid_type) );
    }

    for ( int i = 0; i < nn; ++i ) {
      gen = xmt::uid();
      db.put_object( data_key[i], &gen, sizeof(xmt::uuid_type) );
    }
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( "/tmp/yard" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_perf::put_revisions)
{
    yard::revision revs;
    try
    {
        const int countOfBlocks = 1000;
        const size_t blockSize = 2*1000;

        const size_t sz = (blockSize / sizeof(xmt::uuid_type)) * sizeof(xmt::uuid_type);
        string random_content( sz, 0 );
        for (int i = 0; i < countOfBlocks; ++i)
        {
            for (int j = 0; j < sz; j+= sizeof(xmt::uuid_type))
            {
              xmt::uuid_type b = xmt::uid();
              uninitialized_copy( b.u.b, b.u.b + sizeof(xmt::uuid_type), const_cast<char*>(random_content.data()) + i );
            }
            revs.push(random_content);
        }
    } catch(const std::invalid_argument& err)
    {
        EXAM_ERROR(err.what());
    } catch (const std::logic_error& err)
    {
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

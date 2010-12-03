// -*- C++ -*- Time-stamp: <2010-12-03 16:05:42 ptr>

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

int EXAM_IMPL(yard_perf::put_mess)
{
  const int nn = 1024*1;

  xmt::uuid_type root = xmt::uid_md5( ".HEAD." ); // xmt::uid();
  xmt::uuid_type L = xmt::uid_md5( ".INBOX." );
  xmt::uuid_type l = xmt::uid(); // uid_md5( ".title." );

  string mess( 1000, char(0) );

  try {
    yard::yard db( "/tmp/yard" );

    db.add_manifest( root );
    db.add_manifest( root, L );
    db.add_leaf( L, l, std::string( "Inbox" ) );

    // xmt::uuid_type gen;
    xmt::uuid_type id;

    vector<yard::id_type> data_key;

    data_key.reserve( nn );

    for ( int i = 0; i < nn; ++i ) {
      id = xmt::uid();
      // data_key.push_back( id );

      string mess_put( mess );
      mess_put += string( id );

      db.add_leaf( L, id, mess_put );
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
  yard::yard_ng db;

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

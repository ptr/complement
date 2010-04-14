// -*- C++ -*- Time-stamp: <10/04/14 17:31:49 ptr>

/*
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "yard_test.h"

#include <yard/yard.h>

#include <fstream>
#include <vector>
#include <unistd.h>

// using namespace yard;
using namespace std;

int EXAM_IMPL(yard_test::create)
{
  const int hash_size = 0x200;

  try {
    yard::yard db( "/tmp/yard" );

    fstream f( "/tmp/yard" );

    EXAM_REQUIRE( f.is_open() );
    EXAM_REQUIRE( f.good() );

    uint64_t v;

    for ( int i = 0; i < hash_size; ++i ) {
      f.read( reinterpret_cast<char*>(&v), sizeof(v) );
      EXAM_REQUIRE( !f.fail() );
      EXAM_REQUIRE( v == static_cast<uint64_t>(-1) );
      v = 0;
    }
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }

  try {
    // assume that it run after "create" above
    yard::yard db( "/tmp/yard" );

    fstream f( "/tmp/yard" );

    EXAM_REQUIRE( f.is_open() );
    EXAM_REQUIRE( f.good() );

    uint64_t v;

    for ( int i = 0; i < hash_size; ++i ) {
      f.read( reinterpret_cast<char*>(&v), sizeof(v) );
      EXAM_REQUIRE( !f.fail() );
      EXAM_REQUIRE( v == static_cast<uint64_t>(-1) );
      v = 0;
    }
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( "/tmp/yard" );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::put)
{
  const int nn = 102400;

  vector<xmt::uuid_type> data_put;
  vector<yard::id_type>  data_key;

  data_put.reserve( nn );
  data_key.reserve( nn );

  try {
    yard::yard db( "/tmp/yard" );

    char data[] = "Hello, world!";

    yard::id_type id = db.put_revision( data, sizeof(data) );

    yard::id_type id2 = db.put_revision( data, sizeof(data) );

    EXAM_CHECK( id == id2 );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      // cerr << i << endl;
      gen = xmt::uid();
      data_put.push_back( gen );
      data_key.push_back( db.put_revision( &gen, sizeof(xmt::uuid_type) ) );

      // for ( int j = 0; j <= i; ++j ) {
      //   cerr << ' ' << j << endl;
      //   EXAM_CHECK( db.get( data_key[j] ) == std::string( reinterpret_cast<char*>(&data_put[j].u.b[0]), sizeof(xmt::uuid_type) ) );
      // }
    }

    std::string tmp;

    for ( int i = 0; i < nn; ++i ) {
      tmp = db.get( data_key[i] );

      // cerr << i << ' ' << std::string(*reinterpret_cast<const xmt::uuid_type*>(tmp.data())) << ' ' << std::string(data_put[i]) << endl;
      EXAM_CHECK( db.get( data_key[i] ) == std::string( reinterpret_cast<char*>(&data_put[i].u.b[0]), sizeof(xmt::uuid_type) ) );
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

int EXAM_IMPL(yard_test::put_object)
{
  const int nn = 10240;

  vector<xmt::uuid_type> data_put;
  vector<yard::id_type>  data_key;

  data_put.reserve( nn );
  data_key.reserve( nn );

  try {
    yard::yard db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      // cerr << i << endl;
      gen = xmt::uid();
      data_put.push_back( gen );
      data_key.push_back( db.put_revision( &gen, sizeof(xmt::uuid_type) ) );

      // for ( int j = 0; j <= i; ++j ) {
      //   cerr << ' ' << j << endl;
      //   EXAM_CHECK( db.get( data_key[j] ) == std::string( reinterpret_cast<char*>(&data_put[j].u.b[0]), sizeof(xmt::uuid_type) ) );
      // }
    }

    xmt::uuid_type id;

    for ( int i = 0; i < nn; ++i ) {
      gen = xmt::uid();
      id = xmt::uid();
      db.put_object( id, &gen, sizeof(xmt::uuid_type) );
      gen = xmt::uid();
      db.put_object( id, &gen, sizeof(xmt::uuid_type) );
    }

    std::string tmp;

    for ( int i = 0; i < nn; ++i ) {
      tmp = db.get( data_key[i] );

      // cerr << i << ' ' << std::string(*reinterpret_cast<const xmt::uuid_type*>(tmp.data())) << ' ' << std::string(data_put[i]) << endl;
      EXAM_CHECK( db.get( data_key[i] ) == std::string( reinterpret_cast<char*>(&data_put[i].u.b[0]), sizeof(xmt::uuid_type) ) );
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

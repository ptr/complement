// -*- C++ -*- Time-stamp: <10/04/13 10:32:05 ptr>

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

  // vector<xmt::uuid_type> data_put;
  // vector<yard::id_type>  data_key;

  // data_put.reserve( nn );
  // data_key.reserve( nn );

  try {
    yard::yard db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      // cerr << i << endl;
      gen = xmt::uid();
      // data_put.push_back( gen );
      // data_key.push_back( db.put( &gen, sizeof(xmt::uuid_type) ) );
      db.put( &gen, sizeof(xmt::uuid_type) );

      // for ( int j = 0; j <= i; ++j ) {
      //   cerr << ' ' << j << endl;
      //   EXAM_CHECK( db.get( data_key[j] ) == std::string( reinterpret_cast<char*>(&data_put[j].u.b[0]), sizeof(xmt::uuid_type) ) );
      // }
    }

#if 0
    std::string tmp;

    for ( int i = 0; i < nn; ++i ) {
      tmp = db.get( data_key[i] );

      // cerr << i << ' ' << std::string(*reinterpret_cast<const xmt::uuid_type*>(tmp.data())) << ' ' << std::string(data_put[i]) << endl;
      EXAM_CHECK( db.get( data_key[i] ) == std::string( reinterpret_cast<char*>(&data_put[i].u.b[0]), sizeof(xmt::uuid_type) ) );
    }
#endif
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

  // vector<xmt::uuid_type> data_put;
  vector<yard::id_type>  data_key;

  // data_put.reserve( nn );
  data_key.reserve( nn );

  try {
    yard::yard db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      // cerr << i << endl;
      gen = xmt::uid();
      // data_put.push_back( gen );
      data_key.push_back( db.put( &gen, sizeof(xmt::uuid_type) ) );
      // db.put( &gen, sizeof(xmt::uuid_type) );

      // for ( int j = 0; j <= i; ++j ) {
      //   cerr << ' ' << j << endl;
      //   EXAM_CHECK( db.get( data_key[j] ) == std::string( reinterpret_cast<char*>(&data_put[j].u.b[0]), sizeof(xmt::uuid_type) ) );
      // }
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

  // vector<xmt::uuid_type> data_put;
  // vector<yard::id_type>  data_key;

  // data_put.reserve( nn );
  // data_key.reserve( nn );

  try {
    yard::yard db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      // cerr << i << endl;
      gen = xmt::uid();
      // data_put.push_back( gen );
      // data_key.push_back( db.put( &gen, sizeof(xmt::uuid_type) ) );
      db.put( &gen, sizeof(xmt::uuid_type) );

      // for ( int j = 0; j <= i; ++j ) {
      //   cerr << ' ' << j << endl;
      //   EXAM_CHECK( db.get( data_key[j] ) == std::string( reinterpret_cast<char*>(&data_put[j].u.b[0]), sizeof(xmt::uuid_type) ) );
      // }
    }

#if 0
    std::string tmp;

    for ( int i = 0; i < nn; ++i ) {
      tmp = db.get( data_key[i] );

      // cerr << i << ' ' << std::string(*reinterpret_cast<const xmt::uuid_type*>(tmp.data())) << ' ' << std::string(data_put[i]) << endl;
      EXAM_CHECK( db.get( data_key[i] ) == std::string( reinterpret_cast<char*>(&data_put[i].u.b[0]), sizeof(xmt::uuid_type) ) );
    }
#endif
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

  // vector<xmt::uuid_type> data_put;
  // vector<yard::id_type>  data_key;

  // data_put.reserve( nn );
  // data_key.reserve( nn );

  try {
    yard::yard db( "/tmp/yard" );

    xmt::uuid_type gen;

    for ( int i = 0; i < nn; ++i ) {
      // cerr << i << endl;
      gen = xmt::uid();
      // data_put.push_back( gen );
      // data_key.push_back( db.put( &gen, sizeof(xmt::uuid_type) ) );
      db.put( &gen, sizeof(xmt::uuid_type) );

      // for ( int j = 0; j <= i; ++j ) {
      //   cerr << ' ' << j << endl;
      //   EXAM_CHECK( db.get( data_key[j] ) == std::string( reinterpret_cast<char*>(&data_put[j].u.b[0]), sizeof(xmt::uuid_type) ) );
      // }
    }

#if 0
    std::string tmp;

    for ( int i = 0; i < nn; ++i ) {
      tmp = db.get( data_key[i] );

      // cerr << i << ' ' << std::string(*reinterpret_cast<const xmt::uuid_type*>(tmp.data())) << ' ' << std::string(data_put[i]) << endl;
      EXAM_CHECK( db.get( data_key[i] ) == std::string( reinterpret_cast<char*>(&data_put[i].u.b[0]), sizeof(xmt::uuid_type) ) );
    }
#endif
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

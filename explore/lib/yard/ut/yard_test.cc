// -*- C++ -*- Time-stamp: <2010-11-29 20:05:59 ptr>

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

int EXAM_IMPL(yard_test::revision_in_memory)
{
  yard::revision rev;
  string content0( "01234567890123456789" );
  string content1( "012345678901234567890" );

  try {
    yard::revision_id_type r0 = rev.push( content0 );
    yard::revision_id_type r1 = rev.push( content1 );

    EXAM_CHECK( rev.get( r0 ) == content0 );
    EXAM_CHECK( rev.get( r1 ) == content1 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  
  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::access)
{
  yard::yard_ng db;
  string content0( "01234567890123456789" );
  string content1( "012345678901234567890" );

  try {
    yard::commit_id_type cid = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid );
    db.add( cid, "/one", content0 );
    db.add( cid, "/two", content1 );
    db.close_commit_delta( cid );

    EXAM_CHECK( db.get( cid, "/one" ) == content0 );
    EXAM_CHECK( db.get( cid, "/two" ) == content1 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::linear_commits)
{
  yard::yard_ng db;
  string content0( "01234567890123456789" );
  string content1( "012345678901234567890" );

  try {
    yard::commit_id_type cid = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid );
    db.add( cid, "/one", content0 );
    db.add( cid, "/two", content1 );
    db.close_commit_delta( cid );

    EXAM_CHECK( db.get( cid, "/one" ) == content0 );
    EXAM_CHECK( db.get( cid, "/two" ) == content1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( cid, cid2 );
    string content2( "2" );

    db.add( cid2, "/one", content2 );

    db.close_commit_delta( cid2 );

    EXAM_CHECK( db.get( cid, "/one" ) == content0 );
    EXAM_CHECK( db.get( "/one" ) == content2 );
    EXAM_CHECK( db.get( cid2, "/one" ) == content2 );

    yard::commit_id_type cid3 = xmt::uid();
    
    db.open_commit_delta( cid2, cid3 );
    string content3( "3" );

    db.add( cid3, "/two", content3 );

    db.close_commit_delta( cid3 );

    EXAM_CHECK( db.get( cid, "/one" ) == content0 );
    EXAM_CHECK( db.get( "/one" ) == content2 );
    EXAM_CHECK( db.get( cid2, "/one" ) == content2 );
    EXAM_CHECK( db.get( cid3, "/one" ) == content2 );

    EXAM_CHECK( db.get( cid, "/two" ) == content1 );
    EXAM_CHECK( db.get( "/two" ) == content3 );
    EXAM_CHECK( db.get( cid2, "/two" ) == content1 );
    EXAM_CHECK( db.get( cid3, "/two" ) == content3 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::linear_commits_neg)
{
  yard::yard_ng db;
  string content0( "01234567890123456789" );
  string content1( "012345678901234567890" );

  try {
    yard::commit_id_type cid = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid );
    db.add( cid, "/one", content0 );
    db.add( cid, "/two", content1 );
    db.close_commit_delta( cid );

    EXAM_CHECK( db.get( cid, "/one" ) == content0 );
    EXAM_CHECK( db.get( cid, "/two" ) == content1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( xmt::nil_uuid, cid2 );
    string content2( "2" );

    db.add( cid2, "/one", content2 );

    db.close_commit_delta( cid2 );

    db.get( "/one" ); // throw

    EXAM_ERROR( "more then one head: logic_error exception expected" );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    // EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::create)
{
  const int hash_size = 0x200;

  try {
    yard::underground db( "/tmp/yard" );

    fstream f( "/tmp/yard" );

    EXAM_REQUIRE( f.is_open() );
    EXAM_REQUIRE( f.good() );

    uint64_t v = 100, vv, hoff, hsz, ds;
    bool hoff_section = false, ds_section = false, sz_hash = false, stop_rec = false;

    for ( ; f.good() && v != 0; ) {
      f.read( reinterpret_cast<char*>(&v), sizeof(v) );
      f.read( reinterpret_cast<char*>(&vv), sizeof(vv) );
      EXAM_REQUIRE( !f.fail() );
      switch ( v ) {
        case 0:
          stop_rec = true;
          break;
        case 1:
	  hoff_section = true;
          hoff = vv;
          break;
        case 2:
	  ds_section = true;
          ds = vv;
          break;
        case 3:
	  sz_hash = true;
	  hsz = vv;
          break;
      }
    }

    EXAM_CHECK( hoff_section );
    EXAM_CHECK( ds_section );
    EXAM_CHECK( sz_hash );
    EXAM_CHECK( stop_rec );

    EXAM_CHECK( static_cast<uint64_t>(f.tellg()) == hoff );
    EXAM_CHECK( hsz == hash_size );
    EXAM_CHECK( ds == (hoff + hsz * sizeof(uint64_t)) );

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
    yard::underground db( "/tmp/yard" );

    fstream f( "/tmp/yard" );

    EXAM_REQUIRE( f.is_open() );
    EXAM_REQUIRE( f.good() );

    uint64_t v = 100, vv, hoff, hsz, ds;
    bool hoff_section = false, ds_section = false, sz_hash = false, stop_rec = false;

    for ( ; f.good() && v != 0; ) {
      f.read( reinterpret_cast<char*>(&v), sizeof(v) );
      f.read( reinterpret_cast<char*>(&vv), sizeof(vv) );
      EXAM_REQUIRE( !f.fail() );
      switch ( v ) {
        case 0:
          stop_rec = true;
          break;
        case 1:
	  hoff_section = true;
          hoff = vv;
          break;
        case 2:
	  ds_section = true;
          ds = vv;
          break;
        case 3:
	  sz_hash = true;
	  hsz = vv;
          break;
      }
    }

    EXAM_CHECK( hoff_section );
    EXAM_CHECK( ds_section );
    EXAM_CHECK( sz_hash );
    EXAM_CHECK( stop_rec );

    EXAM_CHECK( static_cast<uint64_t>(f.tellg()) == hoff );
    EXAM_CHECK( hsz == hash_size );
    EXAM_CHECK( ds == (hoff + hsz * sizeof(uint64_t)) );

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
    yard::underground db( "/tmp/yard" );

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
    yard::underground db( "/tmp/yard" );

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

int EXAM_IMPL(yard_test::manifest)
{
  xmt::uuid_type root = xmt::uid_md5( ".HEAD." ); // xmt::uid();
  xmt::uuid_type L = xmt::uid_md5( ".INBOX." );
  xmt::uuid_type l = xmt::uid(); // uid_md5( ".title." );

  try {
    {
      yard::yard db( "/tmp/yard" );

      db.add_manifest( root );
      db.add_manifest( root, L );
      db.add_leaf( L, l, std::string( "Inbox" ) );

      list<yard::id_type> revs;

      db.get_revisions( l, back_inserter(revs) );

      EXAM_CHECK( revs.size() == 1 );
    }

    {
      yard::yard db( "/tmp/yard" );

      // db.add_manifest( root );
      EXAM_CHECK( db.get( l ) == "Inbox" );

      list<yard::id_type> revs;

      db.get_revisions( l, back_inserter(revs) );

      EXAM_CHECK( revs.size() == 1 );
    }

    {
      yard::yard db( "/tmp/yard" );

      // db.add_manifest( root );
      EXAM_CHECK( db.get( l ) == "Inbox" );
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

// -*- C++ -*- Time-stamp: <2011-03-04 11:51:55 ptr>

/*
 * Copyright (c) 2010-2011
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "yard_test.h"

#include <yard/yard.h>
#include "../pack.h"

#include <fstream>
#include <vector>
#include <unistd.h>

// using namespace yard;
using namespace std;

int EXAM_IMPL(yard_test::pack_test)
{
    using namespace yard;

    {
        stringstream ss;
        xmt::uuid_type before;
        before.u.l[1] = before.u.l[1] = 0;

        uuid_packer_exp::pack(ss, before);

        xmt::uuid_type after;

        ss.seekg(0, ios_base::beg);
        uuid_packer_exp::unpack(ss, after);

        EXAM_REQUIRE(before == after);
    }

    const int count = 10000;

    for (int i = 0; i < count; ++i)
    {
        stringstream ss;
        xmt::uuid_type before = xmt::uid();
        before.u.l[1] = 0;

        uuid_packer_exp::pack(ss, before);

        xmt::uuid_type after;

        ss.seekg(0, ios_base::beg);
        uuid_packer_exp::unpack(ss, after);

        EXAM_REQUIRE(before == after);
    }

    for (int i = 0; i < count; ++i)
    {
        stringstream ss;
        xmt::uuid_type before = xmt::uid();

        uuid_packer_exp::pack(ss, before);

        xmt::uuid_type after;

        ss.seekg(0, ios_base::beg);
        uuid_packer_exp::unpack(ss, after);

        EXAM_REQUIRE(before == after);
    }

    for (int i = 0; i < count; ++i)
    {
        stringstream ss;
        uint32_t before = rand();

        varint_packer::pack(ss, before);

        uint32_t after;

        ss.seekg(0, ios_base::beg);
        varint_packer::unpack(ss, after);

        EXAM_REQUIRE(before == after);
    }

    return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::revision)
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

int EXAM_IMPL(yard_test::manifest_from_revision)
{
  yard::revision rev;

  yard::manifest_type m;
  yard::revision_id_type rid1 = xmt::uid();
  yard::revision_id_type rid2 = xmt::uid();

  m["/1"] = rid1;
  m["/2"] = rid2;

  try {
    yard::revision_id_type r0 = rev.push( m );

    yard::manifest_type m_get;

    rev.get_manifest( m_get, r0 );

    EXAM_CHECK( m_get.size() == 2 );
    EXAM_CHECK( m_get["/1"] == m["/1"] );
    EXAM_CHECK( m_get["/2"] == m["/2"] );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::diff_from_revision)
{
  yard::revision rev;

  yard::diff_type d;
  yard::revision_id_type rid1 = xmt::uid();
  yard::revision_id_type rid2 = xmt::uid();
  yard::revision_id_type rid3 = xmt::uid();
  yard::revision_id_type rid4 = xmt::uid();

  d.first["/1"] = rid1;
  d.first["/3"] = rid3;
  d.second["/2"] = rid2;
  d.second["/4"] = rid4;

  try {
    yard::revision_id_type r0 = rev.push( d );

    yard::diff_type d_get;

    rev.get_diff( d_get, r0 );

    EXAM_CHECK( d_get.first.size() == 2 );
    EXAM_CHECK( d_get.second.size() == 2 );
    EXAM_CHECK( d_get.first["/1"] == d.first["/1"] );
    EXAM_CHECK( d_get.first["/3"] == d.first["/3"] );
    EXAM_CHECK( d_get.second["/2"] == d.second["/2"] );
    EXAM_CHECK( d_get.second["/4"] == d.second["/4"] );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::commit_from_revision1)
{
  yard::revision rev;
  yard::commit_id_type cid = xmt::uid();

  yard::commit_node c;

  c.dref = -1;
  c.mid = xmt::uid();

  try {
    yard::revision_id_type r0 = rev.push( c, cid );

    yard::commit_node c_get;

    rev.get_commit( c_get, cid );

    EXAM_CHECK( c_get.dref == -1 );
    EXAM_CHECK( c_get.mid == c.mid );
    EXAM_CHECK( c_get.edge_in.size() == 0 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::commit_from_revision2)
{
  yard::revision rev;
  yard::commit_id_type cid = xmt::uid();
  yard::commit_id_type cid1 = xmt::uid();

  yard::commit_node c;

  c.dref = 0;
  // c.mid = xmt::uid();
  c.delta = new yard::diff_type;

  yard::revision_id_type rid1 = xmt::uid();
  yard::revision_id_type rid2 = xmt::uid();
  yard::revision_id_type rid3 = xmt::uid();
  yard::revision_id_type rid4 = xmt::uid();

  c.delta->first["/1"] = rid1;
  c.delta->first["/3"] = rid3;
  c.delta->second["/2"] = rid2;
  c.delta->second["/4"] = rid4;

  c.edge_in.push_back( xmt::uid() );

  try {
    yard::revision_id_type r0 = rev.push( c, cid );

    yard::commit_node c_get;

    rev.get_commit( c_get, cid );

    EXAM_CHECK( c_get.dref == 0 );
    // EXAM_CHECK( c_get.mid == c.mid );
    EXAM_CHECK( c_get.edge_in.size() == 1 );
    EXAM_CHECK( c_get.delta != 0 );
    EXAM_CHECK( c_get.delta->first["/1"] == c.delta->first["/1"] );
    EXAM_CHECK( c_get.delta->first["/3"] == c.delta->first["/3"] );
    EXAM_CHECK( c_get.delta->second["/2"] == c.delta->second["/2"] );
    EXAM_CHECK( c_get.delta->second["/4"] == c.delta->second["/4"] );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  delete c.delta;

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::access)
{
  yard::yard db;
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
  yard::yard db;
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
  yard::yard db;
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

int EXAM_IMPL(yard_test::diff)
{
  yard::yard db;
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

    yard::diff_type delta = db.diff( cid, cid3 );

    EXAM_CHECK( delta.first.size() == 2 );
    EXAM_CHECK( delta.second.size() == 2 );

    for ( yard::manifest_type::const_iterator i = delta.first.begin(); i != delta.first.end(); ++i ) {
      if ( i->first == "/one" ) {
        EXAM_CHECK( db.get( i->second ) == content0 );
      } else if ( i->first == "/two" ) {
        EXAM_CHECK( db.get( i->second ) == content1 );
      } else {
        EXAM_ERROR( "unexpected" );
      }

      // cerr << '-' << i->first << ' ' << i->second << endl;
    }

    for ( yard::manifest_type::const_iterator i = delta.second.begin(); i != delta.second.end(); ++i ) {
      if ( i->first == "/one" ) {
        EXAM_CHECK( db.get( i->second ) == content2 );
      } else if ( i->first == "/two" ) {
        EXAM_CHECK( db.get( i->second ) == content3 );
      } else {
        EXAM_ERROR( "unexpected" );
      }

      // cerr << '+' << i->first << ' ' << i->second << endl;
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

int EXAM_IMPL(yard_test::fast_merge1)
{
  yard::yard db;

  try {
    yard::commit_id_type cid1 = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    string content1( "1" );

    db.add( cid1, "/one", content1 );
    db.close_commit_delta( cid1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( xmt::nil_uuid, cid2 );

    string content2( "2" );

    db.add( cid2, "/two", content2 );

    db.close_commit_delta( cid2 );

    yard::commit_id_type cid3 = xmt::uid();
    EXAM_CHECK( db.fast_merge( cid3, cid1, cid2 ) == 0 );

    EXAM_CHECK( db.get( cid3, "/one" ) == content1 );
    EXAM_CHECK( db.get( cid3, "/two" ) == content2 );

    EXAM_CHECK( db.get( "/one" ) == content1 );
    EXAM_CHECK( db.get( "/two" ) == content2 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::fast_merge2)
{
  yard::yard db;

  try {
    yard::commit_id_type cid1 = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    string content1( "1" );
    string content2( "2" );

    db.add( cid1, "/one", content1 );
    db.add( cid1, "/two", content2 );

    db.close_commit_delta( cid1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( xmt::nil_uuid, cid2 );

    db.add( cid2, "/two", content2 );

    db.close_commit_delta( cid2 );

    yard::commit_id_type cid3 = xmt::uid();
    EXAM_CHECK( db.fast_merge( cid3, cid1, cid2 ) == 0 );

    EXAM_CHECK( db.get( cid3, "/one" ) == content1 );
    EXAM_CHECK( db.get( cid3, "/two" ) == content2 );

    EXAM_CHECK( db.get( "/one" ) == content1 );
    EXAM_CHECK( db.get( "/two" ) == content2 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::fast_merge3)
{
  yard::yard db;

  try {
    yard::commit_id_type cid1 = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    string content1( "1" );
    string content2( "2" );

    db.add( cid1, "/one", content1 );

    db.close_commit_delta( cid1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( xmt::nil_uuid, cid2 );

    db.add( cid2, "/one", content1 );
    db.add( cid2, "/two", content2 );

    db.close_commit_delta( cid2 );

    yard::commit_id_type cid3 = xmt::uid();
    EXAM_CHECK( db.fast_merge( cid3, cid1, cid2 ) == 0 );

    EXAM_CHECK( db.get( cid3, "/one" ) == content1 );
    EXAM_CHECK( db.get( cid3, "/two" ) == content2 );

    EXAM_CHECK( db.get( "/one" ) == content1 );
    EXAM_CHECK( db.get( "/two" ) == content2 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::fast_merge4)
{
  yard::yard db;

  try {
    yard::commit_id_type cid1 = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    string content1( "1" );
    string content2( "2" );

    db.add( cid1, "/one", content1 );

    db.close_commit_delta( cid1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( cid1, cid2 );

    string content3( "3" );
    string content4( "4" );

    db.del( cid2, "/one" );
    db.add( cid2, "/one", content3 );
    db.del( cid2, "/two" );
    db.add( cid2, "/two", content4 );

    db.close_commit_delta( cid2 );

    string content5( "5" );
    string content6( "6" );

    yard::commit_id_type cid3 = xmt::uid();

    db.open_commit_delta( cid2, cid3 );
    db.del( cid3, "/one" );
    db.add( cid3, "/one", content5 );
    db.close_commit_delta( cid3 );

    yard::commit_id_type cid4 = xmt::uid();

    db.open_commit_delta( cid2, cid4 );
    db.del( cid4, "/two" );
    db.add( cid4, "/two", content6 );
    db.close_commit_delta( cid4 );

    yard::commit_id_type cid5 = xmt::uid();

    EXAM_CHECK( db.fast_merge( cid5, cid3, cid4 ) == 0 );

    EXAM_CHECK( db.get( "/one" ) == content5 );
    EXAM_CHECK( db.get( "/two" ) == content6 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::fast_merge_conflict1)
{
  yard::yard db;

  try {
    yard::commit_id_type cid1 = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    string content1( "1" );
    string content2( "2" );

    db.add( cid1, "/one", content1 );

    db.close_commit_delta( cid1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( xmt::nil_uuid, cid2 );

    db.add( cid2, "/one", content2 );

    db.close_commit_delta( cid2 );

    yard::commit_id_type cid3 = xmt::uid();
    EXAM_CHECK( db.fast_merge( cid3, cid1, cid2 ) == 3 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::heads)
{
  yard::yard db;

  try {
    yard::commit_id_type cid1 = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    string content1( "1" );
    string content2( "2" );

    db.add( cid1, "/one", content1 );

    db.close_commit_delta( cid1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( cid1, cid2 );

    string content3( "3" );
    string content4( "4" );

    db.del( cid2, "/one" );
    db.add( cid2, "/one", content3 );
    db.del( cid2, "/two" );
    db.add( cid2, "/two", content4 );

    db.close_commit_delta( cid2 );

    string content5( "5" );
    string content6( "6" );

    yard::commit_id_type cid3 = xmt::uid();

    db.open_commit_delta( cid2, cid3 );
    db.del( cid3, "/one" );
    db.add( cid3, "/one", content5 );
    db.close_commit_delta( cid3 );

    yard::commit_id_type cid4 = xmt::uid();

    db.open_commit_delta( cid2, cid4 );
    db.del( cid4, "/two" );
    db.add( cid4, "/two", content6 );
    db.close_commit_delta( cid4 );

    std::list<yard::commit_id_type> h;

    db.heads( back_inserter(h) );

    EXAM_REQUIRE( h.size() == 2 );
    EXAM_CHECK( (h.front() == cid3) || (h.front() == cid4) );
    EXAM_CHECK( (h.back() == cid3) || (h.back() == cid4) );
    EXAM_CHECK( h.front() !=  h.back() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::merge1)
{
  yard::yard db;

  try {
    yard::commit_id_type cid1 = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    string content1( "1" );
    string content1_2( "3" );

    db.add( cid1, "/one", content1 );
    db.add( cid1, "/two", content1_2 );
    db.close_commit_delta( cid1 );

    yard::commit_id_type cid2 = xmt::uid();
    
    db.open_commit_delta( xmt::nil_uuid, cid2 );

    string content2( "2" );
    string content2_2( "4" );

    db.add( cid2, "/one", content2_2 );
    db.add( cid2, "/two", content2 );

    db.close_commit_delta( cid2 );

    yard::commit_id_type cid3 = xmt::uid();
    yard::conflicts_list_type cnf;
    EXAM_CHECK( db.merge( cid3, cid1, cid2, cnf ) == 0 );
    EXAM_CHECK( cnf.size() == 2 );

    db.add( cid3, "/one", content1 );
    db.add( cid3, "/two", content2 );
    db.close_commit_delta( cid3 );

    EXAM_CHECK( db.get( cid3, "/one" ) == content1 );
    EXAM_CHECK( db.get( cid3, "/two" ) == content2 );

    EXAM_CHECK( db.get( "/one" ) == content1 );
    EXAM_CHECK( db.get( "/two" ) == content2 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::core_life_cycle)
{
  const char fn[] = "/tmp/btree";

  yard::commit_id_type cid1 = xmt::uid();
  string content1( "1" );
  string content2( "2" );

  try {
    yard::yard db( fn, std::ios_base::trunc );

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    db.add( cid1, "/one", content1 );
    db.add( cid1, "/two", content2 );
    db.close_commit_delta( cid1 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }
  EXAM_MESSAGE( "dump db" );
  try {
    yard::yard db( fn );
    EXAM_CHECK( db.get( cid1, "/one" ) == content1 );
    EXAM_CHECK( db.get( cid1, "/two" ) == content2 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( fn );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::clear_mod_flag)
{
  const char fn[] = "/tmp/btree";

  yard::commit_id_type cid1 = xmt::uid();
  string content1( "1" );
  string content2( "2" );

  try {
    yard::yard db( fn, std::ios_base::trunc );

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    db.add( cid1, "/one", content1 );
    db.add( cid1, "/two", content2 );
    db.close_commit_delta( cid1 );
    db.flush(); // <- flush here and in dtor
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }
  EXAM_MESSAGE( "dump db" );
  try {
    yard::yard db( fn );

    EXAM_CHECK( db.get( cid1, "/one" ) == content1 );
    EXAM_CHECK( db.get( cid1, "/two" ) == content2 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  try {
    yard::yard db( fn );

    EXAM_CHECK( db.get( cid1, "/one" ) == content1 );
    EXAM_CHECK( db.get( cid1, "/two" ) == content2 );
    db.flush(); // <- flush here and in dtor
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  try {
    yard::yard db( fn );

    EXAM_CHECK( db.get( cid1, "/one" ) == content1 );
    EXAM_CHECK( db.get( cid1, "/two" ) == content2 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( fn );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::core_life_cycle_single_leaf)
{
  const char fn[] = "/tmp/btree";

  string content1( "1" );
  string content2( "2" );

  try {
    yard::yard db( fn, std::ios_base::trunc );
    yard::commit_id_type cid1 = xmt::uid();

    db.open_commit_delta( xmt::nil_uuid, cid1 );

    db.add( cid1, "/one", content1 );
    db.add( cid1, "/two", content2 );
    db.close_commit_delta( cid1 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }
  EXAM_MESSAGE( "dump db" );
  try {
    yard::yard db( fn );
    EXAM_CHECK( db.get( "/one" ) == content1 );
    EXAM_CHECK( db.get( "/two" ) == content2 );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }

  unlink( fn );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::not_open_bug1)
{
  const char fn[] = "/tmp/btree";

  string content( 1024, 'a' );
  string name( "/aaaaaa" );

  try {
    yard::commit_id_type cid0 = xmt::nil_uuid;
    yard::commit_id_type cid1 = xmt::uid();

    {
      // Next line indeed not open db, because fn not exist:
      yard::yard db( fn );

      EXAM_CHECK( !db.is_open() );

      cid0 = cid1;
      cid1 = xmt::uid();

      /* Bug: if db not open, block lookup achieve infinite recursion;
         should be std::ios_base::failure exception
       */

      db.open_commit_delta( cid0, cid1 ); // bad: no cid0 commit!

      EXAM_ERROR( "std::ios_base::failure expected" );

      db.add( cid1, name, content );

      db.close_commit_delta( cid1 );
    }
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::logic_error& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::ios_base::failure& err ) {
    EXAM_MESSAGE( "std::ios_base::failure as expected" );
  }

  unlink( fn );

  return EXAM_RESULT;
}

int EXAM_IMPL(yard_test::create)
{
  const char fn[] = "/tmp/btree";

  /* This is a example of use-case:
     "open database; if not existed, create one and open"
   */

  try {
    yard::yard db( fn );

    if ( !db.is_open() /* || !db.good() */ ) {
      db.clear();
      db.open( fn, ios_base::trunc );
    }

    EXAM_CHECK( db.is_open() );
    EXAM_CHECK( db.good() );
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

int EXAM_IMPL(yard_test::commits_chain)
{
  const char fn[] = "/tmp/btree";

  /* Bug: first and second itarations are fine,
     but second don't push some info into BTree
     (i.e. into file on disk), so third iteration
     fail on open_commit_delta.

     Reason: commits chain with diffs was not rollout
     up to full manifest, so id of base manifest
     remains unknown.
   */

  string content( 1024, 'a' );
  string name( "/aaaaaa" );

  try {
    yard::commit_id_type cid0;
    yard::commit_id_type cid1 = xmt::nil_uuid;

    for ( int i = 0; i < 3; ++i ) {
      yard::yard db( fn );

      if ( !db.is_open() /* || !db.good() */ ) {
        db.clear();
        db.open( fn, ios_base::trunc );
      }

      content[512] = 'a' + (i % 0x3f);
      content[513] = 'a' + (i >> 6) % 0x3f;
      content[514] = 'a' + (i >> 12) % 0x3f;

      name[1] = 'a' + (i % 0x3f);
      name[2] = 'a' + (i >> 6) % 0x3f;
      name[3] = 'a' + (i >> 12) % 0x3f;

      cid0 = cid1;
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

int EXAM_IMPL(yard_test::same_content)
{
  const char fn[] = "/tmp/btree";

  /* Bug: can't write into db content, that already present
     in the db. From concepts, no duplicate content in db,
     but attempt to write same content is legal and should
     be successful even if content already present.
   */

  string content( 1024, 'a' );

  try {
    yard::commit_id_type cid0;
    yard::commit_id_type cid1 = xmt::nil_uuid;

    {
      yard::yard db( fn, ios_base::trunc );

      cid0 = cid1;
      cid1 = xmt::uid();

      db.open_commit_delta( cid0, cid1 );

      db.add( cid1, "/name1", content );

      db.close_commit_delta( cid1 );

      EXAM_CHECK( db.is_open() );
      EXAM_CHECK( db.good() );
    }

    EXAM_MESSAGE( "content in db" );

    {
      yard::yard db( fn );

      cid0 = cid1;
      cid1 = xmt::uid();

      db.open_commit_delta( cid0, cid1 );

      db.add( cid1, "/name2", content ); // <<-- same content

      db.close_commit_delta( cid1 );

      EXAM_CHECK( db.is_open() );
      EXAM_CHECK( db.good() );
    }

    EXAM_MESSAGE( "content already in db, ok" );
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

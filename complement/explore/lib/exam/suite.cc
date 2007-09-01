// -*- C++ -*- Time-stamp: <07/09/01 09:09:43 ptr>

/*
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 */

#include <exam/suite.h>
#include <stack>

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <functional>

namespace exam {

using namespace std;
using namespace detail;
using namespace xmt;


int EXAM_IMPL(test_suite::_root_func)
{
  throw init_exception();

  return -1;
}

test_suite::test_suite( const string& name ) :
    _count(0),
    _last_state( 0 ),
    _suite_name( name ),
    local_logger( logger )
{
  _vertices.push_back( std::make_pair( 0, 0 ) );
  _test[0].tc = detail::make_test_case( detail::call( _root_func ) );
  _test[0].state = 0;

  scoped_lock lk( _lock_stack );
  _stack.push( this );
}

test_suite::test_suite( const char *name ) :
    _count(0),
    _last_state( 0 ),
    _suite_name( name ),
    local_logger( logger )
{
  _vertices.push_back( std::make_pair( 0, 0 ) );
  _test[0].tc = detail::make_test_case( detail::call( _root_func ) );
  _test[0].state = 0;

  scoped_lock lk( _lock_stack );
  _stack.push( this );
}

test_suite::~test_suite()
{
  scoped_lock lk( _lock_stack );
  _stack.pop();
  lk.unlock();

  for ( test_case_map_type::iterator i = _test.begin(); i != _test.end(); ++i ) {
    delete i->second.tc;
  }
}

bool test_suite::vertices_compare( test_suite::weight_t l, test_suite::weight_t r )
{
  return l.second < r.second;
}

int test_suite::girdle( test_suite::test_case_type start )
{
  if ( start > _count ) {
    throw std::logic_error( "bad start point" );
  }

  sort( _vertices.begin(), _vertices.end(), vertices_compare );

  vector<weight_t>::iterator from;

  _stat = base_logger::stat();
  for( vector<weight_t>::iterator i = _vertices.begin(); i != _vertices.end(); ++i ) {
    if ( i->first == start ) {
      from = i;
    }
    _test[i->first].state = 0;
  }
  local_logger->begin_ts();
  for( vector<weight_t>::iterator i = from; i != _vertices.end(); ++i ) {
    for( std::list<edge_t>::const_iterator j = _edges.begin(); j != _edges.end(); ++j ) {
      if ( j->second == i->first && _test[j->first].state != 0 ) {
        _test[j->second].state = skip;
      }
    }
    run_test_case( i->first );
  }
  
  local_logger->end_ts();
  local_logger->result( _stat, _suite_name );

  return _stat.failed;
}

test_suite::test_case_type test_suite::add( test_suite::func_type f, const string& name )
{
  vertex_t v = ++_count;
  _edges.push_back( std::make_pair( 0, v ) );
  _vertices.push_back( std::make_pair( v, 1 ) );
  _test[v].tc = detail::make_test_case( detail::call( f ) );
  _test[v].state = 0;
  _test[v].name = name;
  // ++_stat.total;

  return v;
}

test_suite::test_case_type test_suite::add( test_suite::func_type f, const string& name, test_suite::test_case_type depends )
{
  vertex_t v = ++_count;
  if ( depends >= _count ) {
    throw std::logic_error( "bad test dependency" );
  }
  _edges.push_back( std::make_pair( depends, v ) );
  _vertices.push_back( std::make_pair( v, _vertices[depends].second + 1 ) );
  _test[v].tc = detail::make_test_case( detail::call( f ) );
  _test[v].state = 0;
  _test[v].name = name;
  // ++_stat.total;

  return v;
}

int test_suite::flags()
{
  scoped_lock lk( _lock_ll );
  int tmp = local_logger->flags();
  return tmp;
}

bool test_suite::is_trace()
{
  scoped_lock lk( _lock_ll );
  bool tmp = local_logger->is_trace();
  return tmp;
}

int test_suite::flags( int f )
{
  scoped_lock lk( _lock_ll );
  int tmp = local_logger->flags( f );
  return tmp;
}

trivial_logger __trivial_logger_inst( cerr );

base_logger *test_suite::logger = &__trivial_logger_inst;
stack<test_suite *> test_suite::_stack;
mutex test_suite::_lock_stack;
mutex test_suite::_lock_gl;

base_logger *test_suite::set_global_logger( base_logger *new_logger )
{
  scoped_lock glk( _lock_gl );
  base_logger *tmp = logger;
  logger = new_logger;
  scoped_lock lk( _lock_ll );
  if ( tmp == local_logger ) { // if local_logger was identical to logger, switch it too
    local_logger = logger;
  }
  return tmp;
}

base_logger *test_suite::set_logger( base_logger *new_logger )
{
  scoped_lock lk( _lock_ll );
  base_logger *tmp = local_logger;
  local_logger = new_logger;
  return tmp;
}

void test_suite::report( const char *file, int line, bool cnd, const char *expr )
{
  if ( !cnd ) {
    _last_state = fail;
  }
  scoped_lock lk( _lock_ll );
  local_logger->report( file, line, cnd, expr );
}

void test_suite::report_async( const char *file, int line, bool cnd, const char *expr )
{
  scoped_lock lk( _lock_stack );

  if ( _stack.empty() ) {
    throw runtime_error( "stack of test suites empty" );
  }

  _stack.top()->report( file, line, cnd, expr );
}

void test_suite::run_test_case( test_suite::vertex_t v )
{
  try {
    ++_stat.total;
    if ( _test[v].state == 0 ) {
      if ( (*_test[v].tc)( this, 0 ) == 0 ) {
        if ( _last_state == 0 ) {
          ++_stat.passed;
          scoped_lock lk( _lock_ll );
          local_logger->tc( base_logger::pass, _test[v].name );
        } else {
          _test[v].state = fail;
          ++_stat.failed;
          scoped_lock lk( _lock_ll );
          local_logger->tc( base_logger::fail, _test[v].name );
          _last_state = 0;
        }
      } else {
        _test[v].state = fail;
        ++_stat.failed;
        scoped_lock lk( _lock_ll );
        local_logger->tc( base_logger::fail, _test[v].name );
        _last_state = 0;
      }
    } else {
      ++_stat.skipped;
      scoped_lock lk( _lock_ll );
      local_logger->tc( base_logger::skip, _test[v].name );
    }
  }
  catch ( init_exception& ) {
    --_stat.total;
  }
  catch ( ... ) {
    ++_stat.failed;
    _test[v].state = fail;
    scoped_lock lk( _lock_ll );
    local_logger->tc( base_logger::fail, _test[v].name );
  }
}

int test_suite::run( test_suite *, int )
{
  return girdle( 0 );
}


} // namespace exam

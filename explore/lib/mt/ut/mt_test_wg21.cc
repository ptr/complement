// -*- C++ -*-

/*
 * Copyright (c) 2006-2011, 2017, 2020
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "mt_test_wg21.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <shared_mutex>
#include <misc/type_traits.h>
#include <typeinfo>

#include <mt/fork.h>
// for barriers, semaphores, inter-process mutex and conditions:
#include <mt/condition_variable>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <mt/uid.h>

#include <string>
#include <set>

#include <vector>
#include <algorithm>
#include <iterator>

static int val = 0;

void thread_func()
{
  val = 1;
}

void thread_func_int( int v )
{
  val = v;
}

int EXAM_IMPL(mt_test_wg21::thread_call)
{
  val = -1;
 
  std::thread t(thread_func);

  t.join();

  EXAM_CHECK( val == 1 );

  std::thread t2(thread_func_int, 2);

  t2.join();

  EXAM_CHECK( val == 2 );

  val = 0;

  return EXAM_RESULT;
}

static std::mutex lk;

void thread_func2()
{
  std::lock_guard<std::mutex> lock(lk);

  ++val;
}

int EXAM_IMPL(mt_test_wg21::mutex_test)
{
  val = 0;

  std::thread t(thread_func2);

  lk.lock();
  --val;
  lk.unlock();

  t.join();

  EXAM_CHECK( val == 0 );

  std::recursive_mutex rlk;

  rlk.lock();
  rlk.lock(); // shouldn't block here
  rlk.unlock();
  rlk.unlock();

  return EXAM_RESULT;
}

namespace rw_mutex_ns {

int n_threads = 3; 
int n_times = 10000;
int shared_res;
std::shared_mutex _lock_heap;

void run()
{
  for ( int i = 0; i < n_times; ++i ) {
    if ( rand() % 2 ) {
      std::lock_guard<std::shared_mutex> lk(_lock_heap);
      ++shared_res;
    } else {
      std::shared_lock<std::shared_mutex> lk(_lock_heap);
      int tmp = shared_res;
      ++tmp;
    }
  }
}

}

int EXAM_IMPL(mt_test_wg21::mutex_rw_test)
{
  std::vector<std::thread*> thr(rw_mutex_ns::n_threads);

  for ( int i = 0;i < rw_mutex_ns::n_threads; ++i ) {
    thr[i] = new std::thread(rw_mutex_ns::run);
  }

  for ( int i = 0; i < rw_mutex_ns::n_threads; ++i ) {
    thr[i]->join();
    delete thr[i];
  }

  return EXAM_RESULT;
}

static std::tr2::barrier bar;

void thread_func3()
{
  try {
    EXAM_CHECK_ASYNC( val == 0 );

    bar.wait();

    std::lock_guard<std::mutex> lock(lk);

    ++val;
  }
  catch ( std::runtime_error& err ) {
    EXAM_ERROR_ASYNC( err.what() );
  }
}

int EXAM_IMPL(mt_test_wg21::barrier)
{
  val = 0;

  std::thread t(thread_func3);

  EXAM_CHECK( val == 0 );

  bar.wait();

  lk.lock();
  --val;
  lk.unlock();

  t.join();

  EXAM_CHECK( val == 0 );

  return EXAM_RESULT;
}

void thread_func4(std::tr2::semaphore* s)
{
  EXAM_CHECK_ASYNC( val == 1 );

  val = 0;

  s->notify_one();
}

int EXAM_IMPL(mt_test_wg21::semaphore)
{
  std::tr2::semaphore s;

  val = 1;

  s.wait();

  std::thread t(thread_func4, &s);

  s.wait();

  EXAM_CHECK( val == 0 );

  t.join();

  val = 1;

  std::tr2::semaphore s1(0);

  std::thread t1(thread_func4, &s1);

  s1.wait();

  EXAM_CHECK( val == 0 );

  t1.join();

  // notify _before_ wait acceptable:

  s1.notify_one();
  s1.wait();

  return EXAM_RESULT;
}

static std::mutex cond_mtx;
static std::condition_variable cnd;

namespace test_wg21 {

struct true_val
{
  bool operator()() const
    { return (val == 1); }
};

}

void thread_func5()
{
  std::unique_lock<std::mutex> lk(cond_mtx);
  
  val = 1;
  cnd.notify_one();
}

int EXAM_IMPL(mt_test_wg21::condition_var)
{
  val = 0;
  
  std::thread t(thread_func5);
  
  std::unique_lock<std::mutex> lk(cond_mtx);
  
  EXAM_CHECK( cnd.wait_for(lk, std::chrono::milliseconds(500), test_wg21::true_val()));
  
  EXAM_CHECK( val == 1 );
  
  t.join();
  
  val = 0;
  
  return EXAM_RESULT;
}

int EXAM_IMPL(mt_test_wg21::fork)
{
  // trivial fork

  int v = 3;
  try {
    std::tr2::this_thread::fork();

    try {

      // Child code 
      EXAM_CHECK_ASYNC( v == 3 );

      v = 5;
    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    try {
      EXAM_CHECK( child.pid() > 0 );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
      EXAM_CHECK( v == 3 );
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }


  // less trivial fork: check interprocess communication via shared memory

  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  EXAM_REQUIRE( id != -1 );
  // if ( id == -1 ) {
  //   cerr << "Error on shmget" << endl;
  // }
  EXAM_REQUIRE( shmctl( id, IPC_STAT, &ds ) != -1 );
  // if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
  //   cerr << "Error on shmctl" << endl;
  // }
  void *buf = shmat( id, 0, 0 );
  EXAM_REQUIRE( buf != reinterpret_cast<void *>(-1) );
  // if ( buf == reinterpret_cast<void *>(-1) ) {
  //   cerr << "Error on shmat" << endl;
  // }

  int& x = *new( buf ) int(4);

  EXAM_CHECK( x == 4 );

  try {
    std::tr2::this_thread::fork();

    try {

      // Child code 
      EXAM_CHECK_ASYNC( v == 3 );

      v = 5;

      EXAM_CHECK_ASYNC( x == 4 );

      x = 6;
    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    try {
      EXAM_CHECK( child.pid() > 0 );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
      EXAM_CHECK( v == 3 );
      EXAM_CHECK( x == 6 );
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::uidstr)
{
  std::string u1 = xmt::uid_str();

  EXAM_CHECK( !u1.empty() );

  EXAM_CHECK( u1.length() == 36 );

  std::string u2 = xmt::uid_str();

  EXAM_CHECK( !u2.empty() );

  EXAM_CHECK( u2.length() == 36 );

  EXAM_CHECK( u1 != u2 );

  std::set<std::string> cnt;

  for ( int i = 0; i < 100; ++i ) {
    std::string s = xmt::uid_str();

    EXAM_REQUIRE( s.length() == 36 );
    EXAM_REQUIRE( s[8] == '-' );
    EXAM_REQUIRE( s[13] == '-' );
    EXAM_REQUIRE( s[18] == '-' );
    EXAM_REQUIRE( s[23] == '-' );

    EXAM_REQUIRE( s.find_first_not_of( "0123456789abcdef-" ) == std::string::npos );
    EXAM_CHECK( cnt.find( s ) == cnt.end() );
    cnt.insert( s );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::hostidstr)
{
  std::string u1 = xmt::hostid_str();

  EXAM_CHECK( !u1.empty() );

  EXAM_CHECK( u1.length() == 36 );

  std::string u2 = xmt::hostid_str();

  EXAM_CHECK( !u2.empty() );

  EXAM_CHECK( u1 == u2 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::hostid)
{
  xmt::uuid_type u1 = xmt::hostid();
  xmt::uuid_type u2 = xmt::hostid();

  EXAM_CHECK( u1 == u2 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::uid)
{
  xmt::uuid_type u1 = xmt::uid();
  xmt::uuid_type u2 = xmt::uid();

  EXAM_CHECK( u1 != u2 );

  /* Check that UUID version is 4 */  
  EXAM_CHECK( (u1.u.b[6] & 0xf0) == 0x40 );
  /* Check the UUID variant for DCE */
  EXAM_CHECK( (u1.u.b[8] & 0xc0) == 0x80 );

  /* Check that UUID version is 4 */  
  EXAM_CHECK( (u2.u.b[6] & 0xf0) == 0x40 );
  /* Check the UUID variant for DCE */
  EXAM_CHECK( (u2.u.b[8] & 0xc0) == 0x80 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::uidconv)
{
  xmt::uuid_type u1 = xmt::hostid();
  std::string u2 = xmt::hostid_str();

  EXAM_CHECK( static_cast<std::string>(u1) == u2 ); // <-- conversion to string

  std::stringstream s;

  s << u1;

  EXAM_CHECK( s.str() == u2 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::istream)
{
  /* This is illustration for

     istreambuf_iterator not increment g-pointer of istream, so character
     will be extracted twice (or more, under some occasion), so it not work
     as single-pass input iterator.

     https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81857
     (for GNU libstdc++)
   */
  std::stringstream s;
  char b[] = "c2ee3d09-43b3-466d-b490-db35999a22cf";
  char r[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  //          012345678901234567890123456789012345
  //          0         1         2         3
  s << b;
  EXAM_CHECK( !s.fail() );

  EXAM_CHECK( s.tellg() == 0 );
  /*
    https://cplusplus.github.io/LWG/issue2471

    It's unspecified how many times copy_n increments the InputIterator.
    uninitialized_copy_n is specified to increment it exactly n times,
    which means if an istream_iterator is used then the next character
    after those copied is read from the stream and then discarded, losing data.

    I believe all three of Dinkumware, libc++ and libstdc++ implement copy_n
    with n - 1 increments of the InputIterator, which avoids reading and
    discarding a character when used with istream_iterator, but is inconsistent
    with uninitialized_copy_n and causes surprising behaviour with
    istreambuf_iterator instead, because copy_n(in, 2, copy_n(in, 2, out))
    is not equivalent to copy_n(in, 4, out)
   */

  /*
    copy_n return result + n (i.e. increment OutputIterator n times) and
    increment InputIterator max(0, n - 1).
   */
  std::copy_n( std::istreambuf_iterator<char>(s), 36, r );
  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( memcmp(b, r, 36) == 0 );
  // EXAM_CHECK( s.tellg() != 35 );
  char c = 'q';
  std::copy_n( std::istreambuf_iterator<char>(s), 1, &c );
  EXAM_CHECK( std::istreambuf_iterator<char>(s) != std::istreambuf_iterator<char>() ); // see comment above
  EXAM_CHECK( !s.fail() ); // surprise, see comment above
  EXAM_CHECK( c != 'q' );
  EXAM_CHECK( c == 'f' ); // surprise, see comment above

  // Suggested workaround technique:
  std::istreambuf_iterator<char> iis(s);
  ++iis; // <---
  std::copy_n( iis, 1, &c );
  // EXAM_CHECK( s.fail() ); // ?!
  EXAM_CHECK( iis == std::istreambuf_iterator<char>() );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::istream_iterator)
{
  /* test from libstdc++, bug 2627 */
  // bool test __attribute__((unused)) = true;
  const std::string s("free the vieques");

  // 1
  std::string res_postfix;
  std::istringstream iss01(s);
  std::istreambuf_iterator<char> isbufit01(iss01);
  for (std::size_t j = 0; j < s.size(); ++j, isbufit01++)
    res_postfix += *isbufit01;

  // 2
  std::string res_prefix;
  std::istringstream iss02(s);
  std::istreambuf_iterator<char> isbufit02(iss02);
  for (std::size_t j = 0; j < s.size(); ++j, ++isbufit02)
    res_prefix += *isbufit02;

  // 3 mixed
  std::string res_mixed;
  std::istringstream iss03(s);
  std::istreambuf_iterator<char> isbufit03(iss03);
  for (std::size_t j = 0; j < (s.size() / 2); ++j)
    {
      res_mixed += *isbufit03;
      ++isbufit03;
      res_mixed += *isbufit03;
      isbufit03++;
    }

  EXAM_CHECK( res_postfix == res_prefix );
  EXAM_CHECK( res_mixed == res_prefix );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::copy_n)
{
  /* test from libstdc++, bug 50119 */

  std::vector<int> v;
  std::istringstream s("1 2 3 4 5");

  std::copy_n(std::istream_iterator<int>(s), 2, back_inserter(v));
  EXAM_CHECK( v.size() == 2 );
  EXAM_CHECK( v[0] == 1 );
  EXAM_CHECK( v[1] == 2 );

  std::copy_n(std::istream_iterator<int>(s), 2, back_inserter(v));
  EXAM_CHECK( v.size() == 4 );
  EXAM_CHECK( v[0] == 1 );
  EXAM_CHECK( v[1] == 2 );
  EXAM_CHECK( v[2] == 3 );
  EXAM_CHECK( v[3] == 4 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::istream_iterator_ctor)
{
  /* test related to libstdc++ bug 50119 */
  /* check that istream_iterator ctor with stream extract
     value from stream (libstdc++ extract it!)

     See: istream_iterator: unexpected read in ctor
     https://gcc.gnu.org/bugzilla/show_bug.cgi?id=81964

    Standard say:
    
        27.6.1  Class template istream_iterator [istream.iterator]
        1 The class template istream_iterator is an input iterator (27.2.3)
          that reads (using operator>>) successive elements from the input
          stream for which it was constructed.
      ->    After it is constructed, and every time ++ is used, the iterator
      ->    reads and stores a value of T.
        ...
    
        27.6.1.1 istream_iterator constructors and destructor [istream.iterator.cons]
        ...
          istream_iterator(istream_type& s);
      ->  3 Effects: Initializes in_stream with addressof(s). value may be initialized
      ->    during construction or the first time it is referenced.
        4 Postconditions: in_stream == addressof(s).
    
    Current position is "After it is constructed ... the iterator reads and stores
    a value of T.".

    See also http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0738r0.html
  */

  std::istringstream s("1 2");
  std::istream_iterator<int> ii1(s); // read 1
  std::istream_iterator<int> ii2(s); // read 2

  EXAM_CHECK( *ii2 == 2 );
  EXAM_CHECK( *ii1 == 1 );

  return EXAM_RESULT;
}

struct dummy {
    char c;
};

std::istream& operator >>( std::istream& s, dummy& d )
{
  std::istream::sentry __sentry( s ); // skip whitespace

  s.read( &d.c, 1 );

  return s;
}

int EXAM_IMPL(uid_test_wg21::sentry)
{
  dummy d;
  std::stringstream s;

  s << "  " << 'a';
  s >> d;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( d.c == 'a' );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::uid_stream)
{
  xmt::uuid_type u = xmt::uid();

  std::stringstream s;

  s << u;

  EXAM_CHECK( s.str() == std::string(u) );

  xmt::uuid_type r;

  s >> r;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( u == r );

  xmt::uuid_type u2 = xmt::uid();
  xmt::uuid_type u3 = xmt::uid();

  s.clear();

  s << "   " << u2 << ' ' << u3;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( !s.eof() );

  xmt::uuid_type r2, r3;

  s >> r2 >> r3;

  EXAM_CHECK( !s.fail() );
  EXAM_CHECK( u2 == r2 );
  EXAM_CHECK( u3 == r3 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::version)
{
  xmt::uuid_type u = xmt::uid();

  EXAM_CHECK( xmt::uid_version( u ) == 4 );
  EXAM_CHECK( xmt::uid_variant( u ) == 2 );

  xmt::uuid_type umd5 = xmt::uid_md5( u.u.b, 16 );

  EXAM_CHECK( xmt::uid_version( umd5 ) == 3 );
  EXAM_CHECK( xmt::uid_variant( umd5 ) == 2 );

  return EXAM_RESULT;
}

int EXAM_IMPL(mt_test_wg21::pid)
{
  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  EXAM_REQUIRE( id != -1 );
  // if ( id == -1 ) {
  //   cerr << "Error on shmget" << endl;
  // }
  EXAM_REQUIRE( shmctl( id, IPC_STAT, &ds ) != -1 );
  // if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
  //   cerr << "Error on shmctl" << endl;
  // }
  void *buf = shmat( id, 0, 0 );
  EXAM_REQUIRE( buf != reinterpret_cast<void *>(-1) );
  // if ( buf == reinterpret_cast<void *>(-1) ) {
  //   cerr << "Error on shmat" << endl;
  // }

  std::tr2::condition_event_ip& fcnd = *new( buf ) std::tr2::condition_event_ip();
  pid_t my_pid = std::tr2::getpid();

  try {
    std::tr2::this_thread::fork();

    int flag = 0;

    try {

      // Child code 
      EXAM_CHECK_ASYNC_F( my_pid == std::tr2::getppid(), flag );
      *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(std::tr2::condition_event_ip)) = std::tr2::getpid();

      fcnd.notify_one();
    }
    catch ( ... ) {
    }

    exit( flag );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    try {
      EXAM_CHECK( child.pid() > 0 );

      fcnd.wait();

      EXAM_CHECK( *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(std::tr2::condition_event_ip)) == child.pid() );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  (&fcnd)->~__condition_event<true>();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );

  return EXAM_RESULT;
}

// -*- C++ -*- Time-stamp: <06/11/28 09:33:13 ptr>

/*
 *
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <iostream>
#include <mt/xmt.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <sys/shm.h>

using namespace std;
using namespace xmt;

/*
 * The problem:
 *  1. Server listen socket (process A)
 *  2. Client connect to server (process B, server --- process A)
 *  3. Client try to read from socket (from server) and block on it,
 *     due to server write nothing (thread i) [Hmm, really here 
       poll with POLLIN flag and then read]
 *  4. In another thread (thread ii) client call system()
 *  5. Due to fork/execl (in system()) client return from read (step 3)
 *     with 0, i.e. as on close connection
 *
 * The POSIX say:
 *   (execl)
 * If the Asynchronous Input and Output option is supported, any
 * outstanding asynchronous I/O operations may be canceled. Those
 * asynchronous I/O operations that are not canceled will complete
 * as if the exec function had not yet occurred, but any associated
 * signal notifications are suppressed. It is unspecified whether
 * the exec function itself blocks awaiting such I/O completion. In no event,
 * however, will the new process image created by the exec function be affected
 * by the presence of outstanding asynchronous I/O operations at the time
 * the exec function is called. Whether any I/O is cancelled, and which I/O may
 * be cancelled upon exec, is implementation-dependent.
 *
 *  Client open socket  --------- system()
 *                      \
 *                        read...[poll signaled, read -> 0]
 *
 * The same issue on server side: poll that wait POLLIN from clients
 * signaled too.
 *
 */

extern int port;
extern xmt::Mutex pr_lock;

// static __Condition<true> cndf;

class ConnectionProcessor5 // dummy variant
{
  public:
    ConnectionProcessor5( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

ConnectionProcessor5::ConnectionProcessor5( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server seen connection" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();
  
  // cerr << "ConnectionProcessor5::ConnectionProcessor5\n";
  timespec tm;
  tm.tv_sec = 3;
  tm.tv_nsec = 0;
  Thread::delay( &tm );

  int n = 1;
  // cerr << "ConnectionProcessor5::ConnectionProcessor5, write\n";
  s.write( (const char *)&n, sizeof( int ) ).flush();
}

void ConnectionProcessor5::connect( std::sockstream& s )
{
}

void ConnectionProcessor5::close()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server: client close connection" );
  pr_lock.unlock();
}

Thread::ret_code thread_entry( void *par )
{
  Thread::ret_code rt;
  Condition& cnd = *reinterpret_cast<Condition *>(par);

  // sem.wait(); // wait server for listen us
  sockstream sock( "localhost", ::port );
  int buff = 0;
  // cerr << "thread_entry" << endl;
  cnd.set( true );
  // Note: due to this is another process then main, boost can report
  // about errors here, but don't count error it in summary, if it occur!
  BOOST_CHECK( sock.read( (char *)&buff, sizeof(int) ).good() ); // <---- key line
  BOOST_CHECK( buff == 1 );
  // cerr << "Read pass" << endl;
  
  rt.iword = 0;
  return rt;
}

void test_read0()
{
  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  if ( id == -1 ) {
    cerr << "Error on shmget" << endl;
  }
  if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
    cerr << "Error on shmctl" << endl;
  }
  void *buf = shmat( id, 0, 0 );
  if ( buf == reinterpret_cast<void *>(-1) ) {
    cerr << "Error on shmat" << endl;
  }

  Semaphore& sem = *new( buf ) Semaphore( 1, true );

  try {
    // cerr << "** 1" << endl;
    // cndf.set( false );

    xmt::fork();                      // <---- key line
    sem.wait(); // wait server for listen us
    // cerr << "** 2" << endl;

    Condition cnd;
    cnd.set( false );

    xmt::Thread thr( thread_entry, &cnd );

    cnd.try_wait(); // wait for read call

    timespec tm;
    tm.tv_sec = 1;
    tm.tv_nsec = 0;
    Thread::delay( &tm );

    // cerr << "system" << endl;
    system( "echo > /dev/null" );  // <------ key line
    // cerr << "after system" << endl;

    thr.join();
    // cerr << "exit child" << endl;
    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    // cerr << "** 3" << endl;
    sockmgr_stream_MP<ConnectionProcessor5> srv( ::port ); // start server

    // cndf.set( true ); // server wait, I hope
    sem.post();
    int stat;
    // cerr << "wait " << child.pid() << endl;
    waitpid( child.pid(), &stat, 0 );
    // cerr << "close all" << endl;
    srv.close();
    srv.wait();
  }
  (&sem)->~Semaphore();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );
}

class ConnectionProcessor6 // dummy variant
{
  public:
    ConnectionProcessor6( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

static Semaphore *sem2;

ConnectionProcessor6::ConnectionProcessor6( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server seen connection" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  sem2->post();
}

void ConnectionProcessor6::connect( std::sockstream& s )
{
}

void ConnectionProcessor6::close()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server: client close connection" );
  pr_lock.unlock();
}

void test_read0_srv()
{
  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  if ( id == -1 ) {
    cerr << "Error on shmget" << endl;
  }
  if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
    cerr << "Error on shmctl" << endl;
  }
  void *buf = shmat( id, 0, 0 );
  if ( buf == reinterpret_cast<void *>(-1) ) {
    cerr << "Error on shmat" << endl;
  }

  sem2 = new( buf ) Semaphore( 2, true );

  sockmgr_stream_MP<ConnectionProcessor6> srv( ::port );

  {
    // It should work as before system call...
    sockstream s( "localhost", ::port );

    s << "1" << endl;

    BOOST_CHECK( s.good() );

    sem2->wait();
  }

  system( "echo > /dev/null" );  // <------ key line

  {
    // ... as after system call.
    sockstream s( "localhost", ::port );

    s << "1" << endl;

    BOOST_CHECK( s.good() );

    sem2->wait();
  }

  BOOST_CHECK( srv.good() ); // server must correctly process interrupt during system call

  srv.close();

  srv.wait();

  sem2->~Semaphore();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );
}

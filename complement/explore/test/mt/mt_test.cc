// -*- C++ -*- Time-stamp: <06/12/26 10:45:51 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <boost/test/unit_test.hpp>

#include "mt_test.h"

#include <mt/xmt.h>
#include <mt/shm.h>

#include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <iostream>

using namespace std;

using namespace boost::unit_test_framework;
namespace fs = boost::filesystem;

void mt_test::fork()
{
  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  BOOST_REQUIRE( id != -1 );
  // if ( id == -1 ) {
  //   cerr << "Error on shmget" << endl;
  // }
  BOOST_REQUIRE( shmctl( id, IPC_STAT, &ds ) != -1 );
  // if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
  //   cerr << "Error on shmctl" << endl;
  // }
  void *buf = shmat( id, 0, 0 );
  BOOST_REQUIRE( buf != reinterpret_cast<void *>(-1) );
  // if ( buf == reinterpret_cast<void *>(-1) ) {
  //   cerr << "Error on shmat" << endl;
  // }

  xmt::__Condition<true>& fcnd = *new( buf ) xmt::__Condition<true>();
  fcnd.set( false );

  try {
    xmt::fork();

    try {

      // Child code 

    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    try {
      BOOST_CHECK( child.pid() > 0 );

      fcnd.set( true );

      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  (&fcnd)->~__Condition<true>();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );
}

void mt_test::pid()
{
  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  BOOST_REQUIRE( id != -1 );
  // if ( id == -1 ) {
  //   cerr << "Error on shmget" << endl;
  // }
  BOOST_REQUIRE( shmctl( id, IPC_STAT, &ds ) != -1 );
  // if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
  //   cerr << "Error on shmctl" << endl;
  // }
  void *buf = shmat( id, 0, 0 );
  BOOST_REQUIRE( buf != reinterpret_cast<void *>(-1) );
  // if ( buf == reinterpret_cast<void *>(-1) ) {
  //   cerr << "Error on shmat" << endl;
  // }

  xmt::__Condition<true>& fcnd = *new( buf ) xmt::__Condition<true>();
  fcnd.set( false );
  pid_t my_pid = xmt::getpid();

  try {
    xmt::fork();

    try {

      // Child code 
      BOOST_CHECK( my_pid == xmt::getppid() );
      *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(xmt::__Condition<true>)) = xmt::getpid();

      fcnd.set( true );

    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    try {
      BOOST_CHECK( child.pid() > 0 );

      fcnd.try_wait();

      BOOST_CHECK( *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(xmt::__Condition<true>)) == child.pid() );

      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  (&fcnd)->~__Condition<true>();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );
}

void mt_test::shm_segment()
{
  const char fname[] = "/tmp/mt_test.shm";
  try {
    xmt::detail::__shm_alloc<0> seg( 5000, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    BOOST_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    seg.deallocate();
    BOOST_CHECK( seg.address() == reinterpret_cast<void *>(-1) );

    BOOST_REQUIRE( !fs::exists( fname ) );

    seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    BOOST_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    seg.deallocate();
    BOOST_CHECK( seg.address() == reinterpret_cast<void *>(-1) );
    BOOST_CHECK( fs::exists( fname ) ); // well, now I don't remove ref file, because shm segment may be created with another way

    // not exclusive, should pass
    seg.allocate( fname, 1024, xmt::shm_base::create, 0660 );
    BOOST_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    try {
      // This instance has segment in usage, should throw
      seg.allocate( fname, 1024, 0, 0660 );
      BOOST_CHECK( false );
    }
    catch ( xmt::shm_bad_alloc& err ) {
      BOOST_CHECK( true ); // Ok
    }

    /*

    I will treat another instanse (<1>) as interface to another
    segment, so this sample not work.

    try {
      // But this is another instanse, it's ok:
      xmt::detail::__shm_alloc<1> seg1( fname, 1024, 0, 0660 );
      BOOST_CHECK( seg1.address() != reinterpret_cast<void *>(-1) );
      // Don't call seg1.deallocate() here, it destroy 
    }
    catch ( xmt::shm_bad_alloc& err ) {
      BOOST_CHECK( false ); // Fail
    }
    */

    seg.deallocate();
    BOOST_CHECK( seg.address() == reinterpret_cast<void *>(-1) );

    // ----
    try {
      // exclusive, should throw
      seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
      BOOST_CHECK( false ); // Fail, should throw
    }
    catch ( xmt::shm_bad_alloc& err ) {
      BOOST_CHECK( true ); // Ok
    }
    BOOST_CHECK( fs::exists( fname ) );
    // ----

    fs::remove( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}

void mt_test::shm_alloc()
{
  const char fname[] = "/tmp/mt_test.shm";
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( fname, 7000, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    {
      xmt::allocator_shm<char,0> shmall;
      size_t sz = shmall.max_size();
      char *ch1 = shmall.allocate( 3500 );
      BOOST_CHECK( ch1 != 0 );
      char *ch2 = shmall.allocate( 3500 );
      BOOST_CHECK( ch2 != 0 );
      try {
        char *ch3 = shmall.allocate( 8 * 1024 - 7000 );
        BOOST_CHECK( false );
      }
      catch ( xmt::shm_bad_alloc& err ) {
        BOOST_CHECK( true );
      }
      shmall.deallocate( ch1, 3500 );
      ch1 = shmall.allocate( 3500 );
      BOOST_CHECK( ch1 != 0 );
      shmall.deallocate( ch2, 3500 );
      ch2 = shmall.allocate( 3500 );
      BOOST_CHECK( ch2 != 0 );
      shmall.deallocate( ch1, 3500 );
      shmall.deallocate( ch2, 3500 );
      ch1 = shmall.allocate( 7000 );
      BOOST_CHECK( ch1 != 0 );
      shmall.deallocate( ch1, 7000 );
      ch1 = shmall.allocate( sz );
      BOOST_CHECK( ch1 != 0 );
      shmall.deallocate( ch1, sz );
      ch1 = shmall.allocate( 7000 );
      BOOST_CHECK( ch1 != 0 );
      shmall.deallocate( ch1, 7000 );
    }
    seg.deallocate();
    fs::remove( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}


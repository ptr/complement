// -*- C++ -*- Time-stamp: <06/12/18 20:00:15 ptr>

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

#include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>

using namespace boost::unit_test_framework;

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

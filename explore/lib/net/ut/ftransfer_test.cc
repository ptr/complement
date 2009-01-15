// -*- C++ -*- Time-stamp: <09/01/15 17:59:49 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "ftransfer_test.h"

#include <net/ftransfer.h>
#include <mt/date_time>
#include <mt/thread>
#include <mt/uid.h>

#include <misc/tfstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

using namespace stem;

int EXAM_IMPL(ftransfer_test::core)
{
  namespace fs = boost::filesystem;

  std::string name;

  {
    misc::otfstream f( "/tmp/" );
    f << "Hello, world!\n" << endl;

    EXAM_CHECK( f.good() );

    name = f.name();
  }

  EXAM_CHECK( fs::exists( name ) );

  std::string target = "/tmp/";

  target += xmt::uid_str();

  {
    FileRcvMgr receiver;
    FileSndMgr sender;

    receiver.set_prefix( target );

    sender.truncate_path( "/tmp/" );
    sender.sendfile( name, receiver.self_id() );

    std::tr2::this_thread::sleep( std::tr2::milliseconds(400) );
  }

  fs::remove( name );

  name = target + '/' + name.substr( sizeof( "/tmp/" ) - 1 );

  EXAM_CHECK( fs::exists( name ) );

  {
    std::ifstream f( name.c_str() );
    std::string line;

    getline( f, line );
    EXAM_CHECK( line == "Hello, world!" );
    EXAM_CHECK( f.good() );
    getline( f, line );
    EXAM_CHECK( line.empty() );
    EXAM_CHECK( f.good() );
    getline( f, line );
    EXAM_CHECK( line.empty() );
    EXAM_CHECK( f.eof() );
  }

  fs::remove_all( target );

  return EXAM_RESULT;
}

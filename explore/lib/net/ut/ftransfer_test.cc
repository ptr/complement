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
#include <misc/md5.h>

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

string md5file(const char* filename)
{
  FILE *file;
  MD5_CTX context;
  int len;
  unsigned char buffer[1024], digest[16];

  if ((file = fopen (filename, "rb")) == NULL)
    throw runtime_error("can't open file");
    
  MD5Init (&context);
  while (len = fread (buffer, 1, 1024, file))
    MD5Update (&context, buffer, len);
  MD5Final (digest, &context);

  fclose (file);
  
  return string((const char*)digest);
}


int EXAM_IMPL(ftransfer_test::big_file)
{
  namespace fs = boost::filesystem;

  std::string name;
  std::string h1, h2;

  {
    misc::otfstream f( "/tmp/" );
    
    // generating 4 Mb file
    for (int i = 0;i < (1 << 18);++i)
      f << xmt::uid_str();

    EXAM_CHECK( f.good() );

    name = f.name();
  }

  EXAM_CHECK( fs::exists( name ) );
  
  h1 = md5file( name.c_str() );
  
  std::string target = "/tmp/";

  target += xmt::uid_str();

  {
    FileRcvMgr receiver;
    FileSndMgr sender;

    receiver.set_prefix( target );

    sender.truncate_path( "/tmp/" );
    sender.sendfile( name, receiver.self_id() );

    std::tr2::this_thread::sleep( std::tr2::seconds(10) );
  }

  fs::remove( name );

  name = target + '/' + name.substr( sizeof( "/tmp/" ) - 1 );

  EXAM_CHECK( fs::exists( name ) );
  
  h2 = md5file( name.c_str() );
  
  EXAM_CHECK( h1 == h2 );

  fs::remove_all( target );
  
  return EXAM_RESULT;
}

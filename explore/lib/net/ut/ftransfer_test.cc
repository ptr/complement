// -*- C++ -*- Time-stamp: <09/01/24 01:34:01 ptr>

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

#include <stem/EventHandler.h>

using namespace stem;

class FileWatcher :
    public stem::EventHandler
{
  public:
    FileWatcher();
    FileWatcher( const char* );
    ~FileWatcher();

    void file_copied_handler( const stem::Event& );
    
    string file_name;
    
  private:
    DECLARE_RESPONSE_TABLE( FileWatcher, stem::EventHandler );
};

FileWatcher::FileWatcher()
{
  enable();
}

FileWatcher::FileWatcher( const char* info ) :
    EventHandler( info )
{
  enable();
}

FileWatcher::~FileWatcher()
{
  disable();
}

void FileWatcher::file_copied_handler( const stem::Event& ev) {
  file_name = ev.value();
}

DEFINE_RESPONSE_TABLE( FileWatcher )
  EV_EDS( 0, EV_FILE_COPIED, file_copied_handler )
END_RESPONSE_TABLE


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
  
  FileWatcher watcher;

  {
    FileRcvMgr receiver;
    FileSndMgr sender;

    stem_scope scope_r( receiver );
    stem_scope scope_s( sender );

    receiver.set_prefix( target );

    sender.truncate_path( "/tmp/" );
    sender.sendfile( name, receiver.self_id(), watcher.self_id() );

    std::tr2::this_thread::sleep( std::tr2::milliseconds(400) );
  }

  fs::remove( name );
  
  EXAM_CHECK( watcher.file_name == name );

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

static string md5file( const char* filename )
{
  FILE* file;
  MD5_CTX context;
  int len;
  unsigned char buffer[1024], digest[16];

  if ( (file = fopen(filename, "rb")) == NULL ) {
    throw runtime_error("can't open file");
  }
    
  MD5Init(&context);
  while ( len = fread( buffer, 1, 1024, file ) ) {
    MD5Update( &context, buffer, len );
  }
  MD5Final( digest, &context );

  fclose(file);
  
  return string( (const char*)digest, sizeof(digest) );
}


int EXAM_IMPL(ftransfer_test::big_file)
{
  namespace fs = boost::filesystem;

  std::string name;
  std::string h1, h2;

  {
    misc::otfstream f( "/tmp/" );
    
    // generating 9MB file
    for ( int i = 0; i < (1 << 18); ++i ) {
      f << xmt::uid_str(); // 36
    }

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

    stem_scope scope_r( receiver );
    stem_scope scope_s( sender );

    receiver.set_prefix( target );

    sender.truncate_path( "/tmp/" );
    sender.sendfile( name, receiver.self_id() );

    // expected speed more then 10MB/s for local transfer
    std::tr2::this_thread::sleep( std::tr2::seconds(5) );
  }

  fs::remove( name );

  name = target + '/' + name.substr( sizeof( "/tmp/" ) - 1 );

  EXAM_CHECK( fs::exists( name ) );
  
  h2 = md5file( name.c_str() );
  
  EXAM_CHECK( h1 == h2 );

  fs::remove_all( target );
  
  return EXAM_RESULT;
}

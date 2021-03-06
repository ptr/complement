// -*- C++ -*- Time-stamp: <2011-10-07 08:15:06 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "ftransfer.h"
#include <stem/EvPack.h>

#include <iterator>
#include <algorithm>

#include <mt/lfstream>
#include <misc/tfstream>
#include <misc/md5.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/tokenizer.hpp>
#include <stack>

// #include <iostream>

namespace stem {

using namespace std;
using namespace stem;

namespace detail {

class file_receiver :
    public EventHandler
{
  public:
    file_receiver( const std::string& nm, stem::ext_addr_type snd, stem::ext_addr_type remover );
    ~file_receiver();

    void next_chunk( const stem::Event& );
    void final( const stem::Event& );

  private:
    misc::otfstream f;
    std::string name;
    stem::ext_addr_type r;
    MD5_CTX ctx;

    DECLARE_RESPONSE_TABLE( file_receiver, EventHandler );
};

class file_sender :
    public EventHandler
{
  public:
    file_sender( const std::string& name,
                 const std::string& rmprefix,
                 stem::ext_addr_type rcv,
                 stem::ext_addr_type remover,
                 stem::ext_addr_type watcher );
    ~file_sender();

    void ack( const stem::Event& );
    void nac();
    void finilaze();
    void next_chunk( const stem::Event& );

  private:
    std::ilfstream f;
    std::string name;
    stem::ext_addr_type r;
    stem::ext_addr_type w;
    MD5_CTX ctx;

    DECLARE_RESPONSE_TABLE( file_sender, EventHandler );
};

const int STEM_FILE_ACK    = 0x10000;
const int STEM_FILE_NAME   = 0x10001;
const int STEM_FILE_SND_RM = 0x10002;
const int STEM_FILE_NAC    = 0x10003;

const int STEM_NEXT_CHUNK  = 0x10004;
const int STEM_FINAL       = 0x10005;

const int ST_DISABLE       = 0x1;
const int ST_SANITY        = 0x2;

const int ST_CORE_SEND     = 0x1;

file_receiver::file_receiver( const std::string& nm, stem::ext_addr_type snd, stem::ext_addr_type remover ) :
    EventHandler(),
    f(),
    name( nm ),
    r( remover )
{
  EventHandler::enable();
  string::size_type p = nm.rfind( '/' );
  if ( p != string::npos ) {
    string prefix = nm.substr( 0, p + 1 );
    f.open( prefix.c_str() );
  } else {
    f.open( "./" );
  }

  if ( f.is_open() && f.good() ) {
    MD5Init( &ctx );
    PushState( ST_CORE_SEND );

    Event ev( STEM_FILE_ACK );
    ev.dest( snd );
    Send( ev );
  } else {
    EventVoid ev( STEM_FILE_NAC );
    ev.dest( snd );
    Send( ev );

    EventVoid fev( STEM_FILE_SND_RM );
    fev.dest( r );
    Send( fev );
  }
}

file_receiver::~file_receiver()
{
  EventHandler::disable();
  if ( f.is_open() ) {
    string tmp = f.name();
    f.close();
    unlink( tmp.c_str() );
  }
}

void file_receiver::next_chunk( const Event& in )
{
  f.write( in.value().data(), in.value().size() );
  
  if ( !f.fail() ) {
    MD5Update( &ctx, reinterpret_cast<const uint8_t*>(in.value().data()), in.value().size() );
    Event ev( STEM_NEXT_CHUNK );
    ev.dest( in.src() );
    Send( ev );
  }
}

void file_receiver::final( const Event& in )
{
  string tmp = f.name();
  f.close();

  uint8_t buf[16];
  MD5Final( buf, &ctx );

  if ( in.value().size() != 16 || !equal( buf, buf + 16, (const uint8_t*)in.value().data() ) ) { // fail
    unlink( tmp.c_str() );
    EventVoid ev( STEM_FILE_SND_RM );
    ev.dest( r );
    Send( ev );
    
    EventVoid nac( STEM_FILE_NAC );
    nac.dest( in.src() );
    Send( nac );    

    return;
  }

  string back = name + '~';

  unlink( back.c_str() );             // ignore error
  link( name.c_str(), back.c_str() ); // ignore error
  unlink( name.c_str() );             // ignore error
  if ( link( tmp.c_str(), name.c_str() ) == 0 ) {
    unlink( tmp.c_str() );
    // success
    EventVoid ev( STEM_FILE_SND_RM );
    ev.dest( r );
    Send( ev );
    
    EventVoid fin( STEM_FINAL );
    fin.dest( in.src() );
    Send( fin );      
  }
}

DEFINE_RESPONSE_TABLE( file_receiver )
  EV_EDS(ST_CORE_SEND,STEM_NEXT_CHUNK,next_chunk)
  EV_EDS(ST_CORE_SEND,STEM_FINAL,final)
END_RESPONSE_TABLE

file_sender::file_sender( const string& nm,
                          const string& rmprefix,
                          stem::ext_addr_type rcv,
                          stem::ext_addr_type remover,
                          stem::ext_addr_type watcher ) :
    EventHandler(),
    name( nm ),
    f( nm.c_str() ),
    r( remover ),
    w( watcher )
{
  EventHandler::enable();
  if ( rmprefix.empty() ) {
    if ( f.good() ) {
      MD5Init( &ctx );
      f.rdlock();
      Event ev( STEM_FILE_NAME );
      ev.value() = name;
      ev.dest( rcv );
      Send( ev );
    }
  } else {
    string::size_type p = name.find( rmprefix );
  
    if ( p == 0 && f.good() ) {
      MD5Init( &ctx );
      f.rdlock();
      Event ev( STEM_FILE_NAME );
      ev.value() = name.substr( rmprefix.length() );
      ev.dest( rcv );
      Send( ev );
    }
  }
}

file_sender::~file_sender()
{
  EventHandler::disable();
  if ( f.is_open() ) {
    f.unlock();
    f.close();
  }
}

void file_sender::ack( const Event& rsp )
{
  PushState( ST_CORE_SEND );
  next_chunk( rsp );
}

void file_sender::next_chunk( const Event& rsp )
{
  Event ev( STEM_NEXT_CHUNK );

  const int limit = 8 * 1024;

  ev.value().reserve( limit );
  ev.dest( rsp.src() );

  int cnt = 0;
  for ( istreambuf_iterator<char> i(f); i != istreambuf_iterator<char>() && cnt < limit; ++cnt, ++i ) {
    ev.value() += *i;
  }

  if ( ev.value().size() == 0 ) {
    //PopState();
    Event fev( STEM_FINAL );
    fev.dest( rsp.src() );

    fev.value().assign( 16, 0 );
    MD5Final( (uint8_t*)(fev.value().data()), &ctx );

    Send( fev );
  } else {
    Send( ev );

    MD5Update( &ctx, reinterpret_cast<const uint8_t*>(ev.value().data()), ev.value().size() );

    if ( ev.value().size() < limit ) {
      Event fev( STEM_FINAL );
      fev.dest( rsp.src() );

      fev.value().assign( 16, 0 );
      MD5Final( (uint8_t*)fev.value().data(), &ctx );

      Send( fev );
    }
  }
}

void file_sender::nac()
{
  if ( f.is_open() ) {
    f.unlock();
    f.close();
  }

  Event ev( STEM_FILE_SND_RM );

  ev.dest( r );
  Send( ev );
}

void file_sender::finilaze()
{
  PopState();
  
  if ( f.is_open() ) {
    f.unlock();
    f.close();
  }

  {
    Event ev( STEM_FILE_SND_RM );
    ev.value() = name;  
    ev.dest( r );
    Send( ev );
  }
  
  {
    Event reply( EV_FILE_COPIED );
    reply.value() = name;
    reply.dest( w );
    Send( reply );
  }  
}

DEFINE_RESPONSE_TABLE( file_sender )
  EV_EDS(ST_NULL,STEM_FILE_ACK,ack)
  EV_EDS(ST_NULL,STEM_FILE_NAC,nac)
  EV_EDS(ST_CORE_SEND,STEM_NEXT_CHUNK,next_chunk)
  EV_EDS(ST_CORE_SEND,STEM_FINAL,finilaze)
END_RESPONSE_TABLE

} // namespace detail

FileSndMgr::FileSndMgr() :
    EventHandler()
{
}

FileSndMgr::FileSndMgr( const char* info ) :
    EventHandler( info )
{
}

FileSndMgr::FileSndMgr( addr_type id, const char* info ) :
    EventHandler( id, info )
{
}

FileSndMgr::~FileSndMgr()
{
  PushState( detail::ST_SANITY );
  for ( container_type::iterator i = senders.begin(); i != senders.end(); ++i ) {
    delete i->second;
  }
  senders.clear();
}

void FileSndMgr::sendfile( const std::string& name, stem::ext_addr_type to, stem::ext_addr_type watcher )
{
  try {
    detail::file_sender* f = new detail::file_sender( name, tranc_prefix, to, make_pair(EventHandler::domain(),self_id()), watcher );

    senders[f->self_id()] = f;
  }
  catch ( ... ) {
  }
}

void FileSndMgr::finish( const stem::Event& ev )
{
  container_type::iterator i = senders.find( ev.src().second );
  if ( i != senders.end() ) {
    delete i->second;
    senders.erase( i );
  }
}

void FileSndMgr::dummy( const stem::Event& )
{
}

DEFINE_RESPONSE_TABLE( FileSndMgr )
  EV_EDS(ST_NULL,detail::STEM_FILE_SND_RM,finish)
  EV_EDS(detail::ST_SANITY,detail::STEM_FILE_SND_RM,dummy)
END_RESPONSE_TABLE

FileRcvMgr::FileRcvMgr() :
    EventHandler()
{
}

FileRcvMgr::FileRcvMgr( const char* info ) :
    EventHandler( info )
{
}

FileRcvMgr::FileRcvMgr( addr_type id, const char* info ) :
    EventHandler( id, info )
{
}

FileRcvMgr::~FileRcvMgr()
{
  PushState( detail::ST_SANITY );
  for ( container_type::iterator i = receivers.begin(); i != receivers.end(); ++i ) {
    delete i->second;
  }
  receivers.clear();
}

void FileRcvMgr::receive( const stem::Event& ev )
{
  try {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep( "/" );
    tokenizer tokens( ev.value(), sep );
    stack<string> st;
    for ( tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter ) {
      if ( *tok_iter == ".." ) {
        if ( st.empty() ) {
          throw invalid_argument( "attempt to exit outside base catalog" );
        }
        st.pop();
      } else {
        st.push( *tok_iter );
      }
    }

    string nm = prefix;
    if ( (nm.length() == 0) || (nm[nm.length() - 1] != '/') ) {
      if ( (ev.value().length() > 0) && (ev.value()[ev.value().length()-1] != '/') ) {
        nm += '/';
      }
    }
    nm += ev.value();

    if ( provide_dir( nm ) ) {
      detail::file_receiver* f = new detail::file_receiver( nm, ev.src(), make_pair(EventHandler::domain(),self_id()) );

      receivers[f->self_id()] = f;
    }
  }
  catch ( ... ) {
  }
}

void FileRcvMgr::finish( const stem::Event& ev )
{
  container_type::iterator i = receivers.find( ev.src().second );
  if ( i != receivers.end() ) {
    delete i->second;
    receivers.erase( i );
  }
}

void FileRcvMgr::dummy( const stem::Event& )
{
}

bool FileRcvMgr::provide_dir( const string& d )
{
  string::size_type p = d.rfind( '/' );
  if ( p != string::npos ) {
    string dir = d.substr( 0, p );

    struct stat buf;

    if ( stat( dir.c_str(), &buf ) != 0 ) {
      if ( provide_dir( dir ) ) {
        return mkdir( dir.c_str(), 0777 ) == 0;
      }
      return false;
    }
  }
  return true;
}

DEFINE_RESPONSE_TABLE( FileRcvMgr )
  EV_EDS(ST_NULL,detail::STEM_FILE_NAME,receive)
  EV_EDS(ST_NULL,detail::STEM_FILE_SND_RM,finish)
  EV_EDS(detail::ST_SANITY,detail::STEM_FILE_SND_RM,dummy)
END_RESPONSE_TABLE

} // namespace stem

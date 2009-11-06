// -*- C++ -*- Time-stamp: <09/01/15 17:30:22 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __net_ftransfer_h
#define __net_ftransfer_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <stem/EventHandler.h>

#include <map>

namespace stem {

namespace detail {

class file_receiver;
class file_sender;

} // namespace detail

// TODO: find better place and value for it
#define EV_FILE_COPIED 0xF11EC01D

class FileSndMgr :
    public EventHandler
{
  public:
    FileSndMgr();
    explicit FileSndMgr( const char* );
    explicit FileSndMgr( addr_type id, const char* info = 0 );
    ~FileSndMgr();
    
    void set_watcher( stem::addr_type w )
      { watcher = w; }
    void truncate_path( const std::string& p )
      { tranc_prefix = p; }
    void sendfile( const std::string& name, stem::addr_type );

  private:
    void finish( const stem::Event& );
    void dummy( const stem::Event& );

    std::string tranc_prefix;
    stem::addr_type watcher;

    typedef std::map<stem::addr_type, detail::file_sender*> container_type;
    container_type senders;

    DECLARE_RESPONSE_TABLE( FileSndMgr, EventHandler );
};

class FileRcvMgr :
    public EventHandler
{
  public:
    FileRcvMgr();
    explicit FileRcvMgr( const char* );
    explicit FileRcvMgr( addr_type id, const char* info = 0 );
    ~FileRcvMgr();

    void set_prefix( const std::string& p )
      { prefix = p; }

  private:
    void receive( const stem::Event& );
    void finish( const stem::Event& );
    void dummy( const stem::Event& );

    bool provide_dir( const std::string& );

    std::string prefix;

    typedef std::map<stem::addr_type, detail::file_receiver*> container_type;
    container_type receivers;

    DECLARE_RESPONSE_TABLE( FileRcvMgr, EventHandler );
};

} // namespace stem

#endif // __net_ftransfer_h

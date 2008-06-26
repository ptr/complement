// -*- C++ -*- Time-stamp: <06/10/03 16:16:41 ptr>

#include <stem/EventHandler.h>
#include <stem/Names.h>
#include <list>
#include <mt/xmt.h>

namespace test {

class NsWrapper :
        public stem::EventHandler
{
  public:
    NsWrapper();
    void ask_names( stem::addr_type );
    const std::list<stem::NameRecord>& names();

  private:
    void names_list( const stem::NameRecord& );
    void names_name( const stem::NameRecord& );

    std::list<stem::NameRecord> lst;
    xmt::Condition lcnd;

    DECLARE_RESPONSE_TABLE( NsWrapper, stem::EventHandler );
};

class MProcessor :
        public stem::EventHandler
{
  public:
    MProcessor( const char *srv_name );

    void send( stem::addr_type, const char *msg );
    void send( stem::addr_type, const char *msg, size_t len );

    const char *get();
    const char *getbinary( unsigned& );
    const char *get( unsigned tv_sec, unsigned tv_nsec );

  private:
    void receive( const stem::Event& ev );

    std::list<stem::Event> income_queue;
    xmt::Mutex lock;
    stem::Event last;
    xmt::Condition cnd;

    DECLARE_RESPONSE_TABLE( MProcessor, stem::EventHandler );
};

} // namespace test

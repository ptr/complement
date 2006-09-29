// -*- C++ -*- Time-stamp: <06/09/29 18:36:51 ptr>

#include <stem/EventHandler.h>
#include <list>
#include <mt/xmt.h>

namespace test {

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

    DECLARE_RESPONSE_TABLE( test::MProcessor, stem::EventHandler );
};

} // namespace test

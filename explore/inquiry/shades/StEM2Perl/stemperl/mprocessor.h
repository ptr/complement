
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

  private:
    void receive( const stem::Event& ev );

    std::list<stem::Event> income_queue;
    xmt::Mutex lock;
    stem::Event last;

    DECLARE_RESPONSE_TABLE( MProcessor, stem::EventHandler );
};

} // namespace test

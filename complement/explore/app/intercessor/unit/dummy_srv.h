// -*- C++ -*- Time-stamp: <07/03/07 14:56:46 ptr>

#ifndef __dummy_srv_h
#define __dummy_srv_h

#include <mt/xmt.h>
#include <sockios/sockstream>

namespace test {

class DummyHttpSrv
{
  public:
    DummyHttpSrv( std::sockstream& );

    void connect( std::sockstream& );
    void close();

    static xmt::condition cnd;
};

class DummyHttpSrvNeg
{
  public:
    DummyHttpSrvNeg( std::sockstream& );

    void connect( std::sockstream& );
    void close();

    static xmt::condition cnd;
};

} // namespace test

#endif // __dummy_srv_h

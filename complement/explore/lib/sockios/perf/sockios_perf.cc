// -*- C++ -*- Time-stamp: <07/09/10 22:39:41 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_perf.h"
#include <exam/suite.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

using namespace std;

template <int N, int S>
class SrvR
{
  public:
    SrvR( sockstream& )
      { }

    ~SrvR()
      { }

    void connect( sockstream& s )
      {
        s.read( buf, S );
        EXAM_CHECK_ASYNC( s.good() );
      }

    void close()
      { }

  private:
    char buf[S];
};

sockmgr_stream_MP<SrvR<32,1638400> > *srv0;
sockmgr_stream_MP<SrvR<1024,51200> > *srv1;
sockmgr_stream_MP<SrvR<4096,12800> > *srv2;
sockmgr_stream_MP<SrvR<6400,8192> >  *srv3;
sockmgr_stream_MP<SrvR<12800,4096> > *srv4;
sockmgr_stream_MP<SrvR<25600,2048> > *srv5;
sockmgr_stream_MP<SrvR<51200,1024> > *srv6;
sockmgr_stream_MP<SrvR<102400,512> > *srv7;
sockmgr_stream_MP<SrvR<204800,256> > *srv8;
sockmgr_stream_MP<SrvR<409600,128> > *srv9;
sockmgr_stream_MP<SrvR<819200,64> >  *srv10;

sockios_perf_SrvR::sockios_perf_SrvR()
{
  srv0 = new sockmgr_stream_MP<SrvR<32,1638400> >( 6480 );
  srv1 = new sockmgr_stream_MP<SrvR<1024,51200> >( 6481 );
  srv2 = new sockmgr_stream_MP<SrvR<4096,12800> >( 6482 );
  srv3 = new sockmgr_stream_MP<SrvR<6400,8192> >( 6483 );
  srv4 = new sockmgr_stream_MP<SrvR<12800,4096> >( 6484 );
  srv5 = new sockmgr_stream_MP<SrvR<25600,2048> >( 6485 );
  srv6 = new sockmgr_stream_MP<SrvR<51200,1024> >( 6486 );
  srv7 = new sockmgr_stream_MP<SrvR<102400,512> >( 6487 );
  srv8 = new sockmgr_stream_MP<SrvR<204800,256> >( 6488 );
  srv9 = new sockmgr_stream_MP<SrvR<409600,128> >( 6489 );
  srv10 = new sockmgr_stream_MP<SrvR<819200,64> >( 6490 );
}

sockios_perf_SrvR::~sockios_perf_SrvR()
{
  srv0->close(); srv0->wait();
  delete srv0;
  srv1->close(); srv1->wait();
  delete srv1;
  srv2->close(); srv2->wait();
  delete srv2;
  srv3->close(); srv3->wait();
  delete srv3;
  srv4->close(); srv4->wait();
  delete srv4;
  srv5->close(); srv5->wait();
  delete srv5;
  srv6->close(); srv6->wait();
  delete srv6;
  srv7->close(); srv7->wait();
  delete srv7;
  srv8->close(); srv8->wait();
  delete srv8;
  srv9->close(); srv9->wait();
  delete srv9;
  srv10->close(); srv10->wait();
  delete srv10;
}

template <class SRV, int N, int S, int P>
int block_write( SRV& srv )
{
  int flag = 0;

  EXAM_CHECK_ASYNC_F( srv.good() && srv.is_open(), flag );
  sockstream s( "localhost", P );
  char buf[S];
  // fill( buf, buf + S, ' ' );
  for ( int i = 0; i < N; ++i ) {
    s.write( buf, S ).flush();
    EXAM_CHECK_ASYNC_F( s.good(), flag );
  }

  return flag;
}

int EXAM_IMPL(sockios_perf_SrvR::r1)
{
  return block_write<sockmgr_stream_MP<SrvR<32,1638400> >, 32, 1638400, 6480>( *srv0 );
}

int EXAM_IMPL(sockios_perf_SrvR::r2)
{
  return block_write<sockmgr_stream_MP<SrvR<1024,51200> >, 1024, 51200, 6481>( *srv1 );
}

int EXAM_IMPL(sockios_perf_SrvR::r3)
{
  return block_write<sockmgr_stream_MP<SrvR<4096,12800> >, 4096, 12800, 6482>( *srv2 );
}

int EXAM_IMPL(sockios_perf_SrvR::r4)
{
  return block_write<sockmgr_stream_MP<SrvR<6400,8192> >, 6400, 8192, 6483>( *srv3 );
}

int EXAM_IMPL(sockios_perf_SrvR::r5)
{
  return block_write<sockmgr_stream_MP<SrvR<12800,4096> >, 12800, 4096, 6484>( *srv4 );
}

int EXAM_IMPL(sockios_perf_SrvR::r6)
{
  return block_write<sockmgr_stream_MP<SrvR<25600,2048> >, 25600, 2048, 6485>( *srv5 );
}

int EXAM_IMPL(sockios_perf_SrvR::r7)
{
  return block_write<sockmgr_stream_MP<SrvR<51200,1024> >, 51200, 1024, 6486>( *srv6 );
}

int EXAM_IMPL(sockios_perf_SrvR::r8)
{
  return block_write<sockmgr_stream_MP<SrvR<102400,512> >, 102400, 512, 6487>( *srv7 );
}

int EXAM_IMPL(sockios_perf_SrvR::r9)
{
  return block_write<sockmgr_stream_MP<SrvR<204800,256> >, 204800, 256, 6488>( *srv8 );
}

int EXAM_IMPL(sockios_perf_SrvR::r10)
{
  return block_write<sockmgr_stream_MP<SrvR<409600,128> >, 409600, 128, 6489>( *srv9 );
}

int EXAM_IMPL(sockios_perf_SrvR::r11)
{
  return block_write<sockmgr_stream_MP<SrvR<819200,64> >, 819200, 64, 6490>( *srv10 );
}

template <int N, int S>
class SrvW
{
  public:
    SrvW( sockstream& s )
      {
        for ( int i = 0; i < N; ++i ) {
          s.write( buf, S ).flush();
          EXAM_CHECK_ASYNC( s.good() );
        }
      }

    ~SrvW()
      { }

    void connect( sockstream& )
      { }

    void close()
      { }

  private:
    char buf[S];
};

sockmgr_stream_MP<SrvW<32,1638400> > *srvW0;
sockmgr_stream_MP<SrvW<1024,51200> > *srvW1;
sockmgr_stream_MP<SrvW<4096,12800> > *srvW2;
sockmgr_stream_MP<SrvW<6400,8192> >  *srvW3;
sockmgr_stream_MP<SrvW<12800,4096> > *srvW4;
sockmgr_stream_MP<SrvW<25600,2048> > *srvW5;
sockmgr_stream_MP<SrvW<51200,1024> > *srvW6;
sockmgr_stream_MP<SrvW<102400,512> > *srvW7;
sockmgr_stream_MP<SrvW<204800,256> > *srvW8;
sockmgr_stream_MP<SrvW<409600,128> > *srvW9;
sockmgr_stream_MP<SrvW<819200,64> >  *srvW10;

sockios_perf_SrvW::sockios_perf_SrvW()
{
  srvW0 = new sockmgr_stream_MP<SrvW<32,1638400> >( 6480 );
  srvW1 = new sockmgr_stream_MP<SrvW<1024,51200> >( 6481 );
  srvW2 = new sockmgr_stream_MP<SrvW<4096,12800> >( 6482 );
  srvW3 = new sockmgr_stream_MP<SrvW<6400,8192> >( 6483 );
  srvW4 = new sockmgr_stream_MP<SrvW<12800,4096> >( 6484 );
  srvW5 = new sockmgr_stream_MP<SrvW<25600,2048> >( 6485 );
  srvW6 = new sockmgr_stream_MP<SrvW<51200,1024> >( 6486 );
  srvW7 = new sockmgr_stream_MP<SrvW<102400,512> >( 6487 );
  srvW8 = new sockmgr_stream_MP<SrvW<204800,256> >( 6488 );
  srvW9 = new sockmgr_stream_MP<SrvW<409600,128> >( 6489 );
  srvW10 = new sockmgr_stream_MP<SrvW<819200,64> >( 6490 );
}

sockios_perf_SrvW::~sockios_perf_SrvW()
{
  srvW0->close(); srvW0->wait();
  delete srvW0;
  srvW1->close(); srvW1->wait();
  delete srvW1;
  srvW2->close(); srvW2->wait();
  delete srvW2;
  srvW3->close(); srvW3->wait();
  delete srvW3;
  srvW4->close(); srvW4->wait();
  delete srvW4;
  srvW5->close(); srvW5->wait();
  delete srvW5;
  srvW6->close(); srvW6->wait();
  delete srvW6;
  srvW7->close(); srvW7->wait();
  delete srvW7;
  srvW8->close(); srvW8->wait();
  delete srvW8;
  srvW9->close(); srvW9->wait();
  delete srvW9;
  srvW10->close(); srvW10->wait();
  delete srvW10;
}

template <class SRV, int N, int S, int P>
int block_read( SRV& srv )
{
  int flag = 0;

  EXAM_CHECK_ASYNC_F( srv.good() && srv.is_open(), flag );
  sockstream s( "localhost", P );
  char buf[S];
  // fill( buf, buf + S, ' ' );
  for ( int i = 0; i < N; ++i ) {
    s.read( buf, S );
    EXAM_CHECK_ASYNC_F( s.good(), flag );
  }

  return flag;
}

int EXAM_IMPL(sockios_perf_SrvW::r1)
{
  return block_read<sockmgr_stream_MP<SrvW<32,1638400> >, 32, 1638400, 6480>( *srvW0 );
}

int EXAM_IMPL(sockios_perf_SrvW::r2)
{
  return block_read<sockmgr_stream_MP<SrvW<1024,51200> >, 1024, 51200, 6481>( *srvW1 );
}

int EXAM_IMPL(sockios_perf_SrvW::r3)
{
  return block_read<sockmgr_stream_MP<SrvW<4096,12800> >, 4096, 12800, 6482>( *srvW2 );
}

int EXAM_IMPL(sockios_perf_SrvW::r4)
{
  return block_read<sockmgr_stream_MP<SrvW<6400,8192> >, 6400, 8192, 6483>( *srvW3 );
}

int EXAM_IMPL(sockios_perf_SrvW::r5)
{
  return block_read<sockmgr_stream_MP<SrvW<12800,4096> >, 12800, 4096, 6484>( *srvW4 );
}

int EXAM_IMPL(sockios_perf_SrvW::r6)
{
  return block_read<sockmgr_stream_MP<SrvW<25600,2048> >, 25600, 2048, 6485>( *srvW5 );
}

int EXAM_IMPL(sockios_perf_SrvW::r7)
{
  return block_read<sockmgr_stream_MP<SrvW<51200,1024> >, 51200, 1024, 6486>( *srvW6 );
}

int EXAM_IMPL(sockios_perf_SrvW::r8)
{
  return block_read<sockmgr_stream_MP<SrvW<102400,512> >, 102400, 512, 6487>( *srvW7 );
}

int EXAM_IMPL(sockios_perf_SrvW::r9)
{
  return block_read<sockmgr_stream_MP<SrvW<204800,256> >, 204800, 256, 6488>( *srvW8 );
}

int EXAM_IMPL(sockios_perf_SrvW::r10)
{
  return block_read<sockmgr_stream_MP<SrvW<409600,128> >, 409600, 128, 6489>( *srvW9 );
}

int EXAM_IMPL(sockios_perf_SrvW::r11)
{
  return block_read<sockmgr_stream_MP<SrvW<819200,64> >, 819200, 64, 6490>( *srvW10 );
}

template <int N, int S>
class SrvRW
{
  public:
    SrvRW( sockstream& )
      { }

    ~SrvRW()
      { }

    void connect( sockstream& s )
      {
        s.read( buf, S );
        EXAM_CHECK_ASYNC( s.good() );
        s.write( buf, S ).flush();
        EXAM_CHECK_ASYNC( s.good() );
      }

    void close()
      { }

  private:
    char buf[S];
};

sockmgr_stream_MP<SrvRW<32,1638400> > *srvRW0;
sockmgr_stream_MP<SrvRW<1024,51200> > *srvRW1;
sockmgr_stream_MP<SrvRW<4096,12800> > *srvRW2;
sockmgr_stream_MP<SrvRW<6400,8192> >  *srvRW3;
sockmgr_stream_MP<SrvRW<12800,4096> > *srvRW4;
sockmgr_stream_MP<SrvRW<25600,2048> > *srvRW5;
sockmgr_stream_MP<SrvRW<51200,1024> > *srvRW6;
sockmgr_stream_MP<SrvRW<102400,512> > *srvRW7;
sockmgr_stream_MP<SrvRW<204800,256> > *srvRW8;
sockmgr_stream_MP<SrvRW<409600,128> > *srvRW9;
sockmgr_stream_MP<SrvRW<819200,64> >  *srvRW10;

sockios_perf_SrvRW::sockios_perf_SrvRW()
{
  srvRW0 = new sockmgr_stream_MP<SrvRW<32,1638400> >( 6480 );
  srvRW1 = new sockmgr_stream_MP<SrvRW<1024,51200> >( 6481 );
  srvRW2 = new sockmgr_stream_MP<SrvRW<4096,12800> >( 6482 );
  srvRW3 = new sockmgr_stream_MP<SrvRW<6400,8192> >( 6483 );
  srvRW4 = new sockmgr_stream_MP<SrvRW<12800,4096> >( 6484 );
  srvRW5 = new sockmgr_stream_MP<SrvRW<25600,2048> >( 6485 );
  srvRW6 = new sockmgr_stream_MP<SrvRW<51200,1024> >( 6486 );
  srvRW7 = new sockmgr_stream_MP<SrvRW<102400,512> >( 6487 );
  srvRW8 = new sockmgr_stream_MP<SrvRW<204800,256> >( 6488 );
  srvRW9 = new sockmgr_stream_MP<SrvRW<409600,128> >( 6489 );
  srvRW10 = new sockmgr_stream_MP<SrvRW<819200,64> >( 6490 );
}

sockios_perf_SrvRW::~sockios_perf_SrvRW()
{
  srvRW0->close(); srvRW0->wait();
  delete srvRW0;
  srvRW1->close(); srvRW1->wait();
  delete srvRW1;
  srvRW2->close(); srvRW2->wait();
  delete srvRW2;
  srvRW3->close(); srvRW3->wait();
  delete srvRW3;
  srvRW4->close(); srvRW4->wait();
  delete srvRW4;
  srvRW5->close(); srvRW5->wait();
  delete srvRW5;
  srvRW6->close(); srvRW6->wait();
  delete srvRW6;
  srvRW7->close(); srvRW7->wait();
  delete srvRW7;
  srvRW8->close(); srvRW8->wait();
  delete srvRW8;
  srvRW9->close(); srvRW9->wait();
  delete srvRW9;
  srvRW10->close(); srvRW10->wait();
  delete srvRW10;
}

template <class SRV, int N, int S, int P>
int block_write_read( SRV& srv )
{
  int flag = 0;

  EXAM_CHECK_ASYNC_F( srv.good() && srv.is_open(), flag );
  sockstream s( "localhost", P );
  char buf[S];
  // fill( buf, buf + S, ' ' );
  for ( int i = 0; i < N; ++i ) {
    s.write( buf, S ).flush();
    EXAM_CHECK_ASYNC_F( s.good(), flag );
    s.read( buf, S );
    EXAM_CHECK_ASYNC_F( s.good(), flag );
  }

  return flag;
}

int EXAM_IMPL(sockios_perf_SrvRW::r1)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<32,1638400> >, 32, 1638400, 6480>( *srvRW0 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r2)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<1024,51200> >, 1024, 51200, 6481>( *srvRW1 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r3)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<4096,12800> >, 4096, 12800, 6482>( *srvRW2 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r4)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<6400,8192> >, 6400, 8192, 6483>( *srvRW3 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r5)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<12800,4096> >, 12800, 4096, 6484>( *srvRW4 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r6)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<25600,2048> >, 25600, 2048, 6485>( *srvRW5 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r7)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<51200,1024> >, 51200, 1024, 6486>( *srvRW6 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r8)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<102400,512> >, 102400, 512, 6487>( *srvRW7 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r9)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<204800,256> >, 204800, 256, 6488>( *srvRW8 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r10)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<409600,128> >, 409600, 128, 6489>( *srvRW9 );
}

int EXAM_IMPL(sockios_perf_SrvRW::r11)
{
  return block_write_read<sockmgr_stream_MP<SrvRW<819200,64> >, 819200, 64, 6490>( *srvRW10 );
}

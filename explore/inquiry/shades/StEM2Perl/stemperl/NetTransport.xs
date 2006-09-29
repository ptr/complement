#ifdef __cplusplus
extern "C" {
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"
#undef do_open
#undef do_close
#undef apply
#undef ref

#ifdef __cplusplus
}
#endif

#undef list

#include <stem/NetTransport.h>
#include "mprocessor.h"
// #include <iostream>

using namespace std;
using namespace stem;

class StEM_NetMgr
{
public:
        NetTransportMgr* mgr;

        StEM_NetMgr() {
          this->mgr = new NetTransportMgr();
        }

        ~StEM_NetMgr() {
          /* delete mgr; */
        }

        unsigned open( const char *hostname, int port ) {
          return mgr->open( hostname, port );
          // cout << mgr->ns() << endl;
          // return (unsigned)mgr;
        }

        void close() {
          mgr->close();
        }

        int join() {
          return mgr->join();
        }
};

class mprocessor
{
public:
        test::MProcessor* prc;

        mprocessor( const char* info )
          { this->prc = new test::MProcessor( info ); }

        void send( unsigned a, const char *m )
          { prc->send( a, m ); }

        void sendbinary( unsigned a, const char *m, unsigned l )
          { prc->send( a, m, l ); }

};

// ___________________
// C-Perl
MODULE = stem::NetTransport     PACKAGE = stem::NetTransport

StEM_NetMgr *
StEM_NetMgr::new()

void
StEM_NetMgr::DESTROY()

unsigned
StEM_NetMgr::open( const char *hostname, int port )

void
StEM_NetMgr::close()

int
StEM_NetMgr::join()

MODULE = stem::NetTransport     PACKAGE = stem::mprocessor

mprocessor *
mprocessor::new( const char* info )

void
mprocessor::DESTROY()

void
mprocessor::send( unsigned a, const char *m )

void
mprocessor::sendbinary( unsigned a, const char *m, unsigned l )

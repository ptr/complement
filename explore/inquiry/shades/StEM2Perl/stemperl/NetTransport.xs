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

        const char *get()
          {
            const char *str = prc->get();
            if ( str != 0 ) {
              return str;
            }
            croak( "No message" );
          }

        const char *getbinary( unsigned *len )
          {
            const char *str = prc->getbinary( *len );
            if ( str != 0 ) {
              return str;
            }
            croak( "No message" );
          }

        const char *get_with_timeout( unsigned sec, unsigned nsec )
          {
            const char *str = prc->get( sec, nsec );
            if ( str != 0 ) {
              return str;
            }
            croak( "No message" );
          }

        unsigned self_id() const
          { return prc->self_id(); }

};

// ___________________
// C-Perl
MODULE = stem     PACKAGE = stem::NetTransport

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

MODULE = stem     PACKAGE = stem::mprocessor

mprocessor *
mprocessor::new( const char* info )

void
mprocessor::DESTROY()

void
mprocessor::send( unsigned a, const char *m )

void
mprocessor::sendbinary( unsigned a, const char *m, unsigned l )

const char *
mprocessor::get()

const char *
mprocessor::getbinary( unsigned& len )

const char *
mprocessor::get_with_timeout( unsigned sec, unsigned nsec )

unsigned
mprocessor::self_id()

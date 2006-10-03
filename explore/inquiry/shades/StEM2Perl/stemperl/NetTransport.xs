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
        test::NsWrapper* wrp;

        StEM_NetMgr()
          {
            this->mgr = new NetTransportMgr();
            this->wrp = new test::NsWrapper();
          }

        ~StEM_NetMgr()
           {
             /* delete mgr; */
             /* delete wrp; */
           }

        unsigned open( const char *hostname, int port )
          {
            unsigned zero = mgr->open( hostname, port );
            wrp->ask_names( mgr->ns() );
            return zero;
          }

        bool good()
          { return mgr->good(); }

        bool bad()
          { return mgr->bad(); }

        bool fail()
          { return mgr->fail(); }

        bool is_open()
          { return mgr->is_open(); }

        void close()
          { mgr->close(); }

        int join()
          { return mgr->join(); }
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

void
StEM_NetMgr::names()
PPCODE:
        std::list<stem::NameRecord> l = THIS->wrp->names();
        // HV * rh = (HV *)sv_2mortal( (SV *)newHV() );
        for ( std::list<stem::NameRecord>::const_iterator i = l.begin(); i != l.end(); ++i ) {
          // hv_store( rh, newSVnv(i->addr), 0, i->record.c_str(), i->record.size() );
          XPUSHs( sv_2mortal(newSVnv(i->addr)) );
          XPUSHs( sv_2mortal(newSVpv(i->record.c_str(),i->record.size())) );
        }
        // XPUSHs( (SV *)rh );

bool
StEM_NetMgr::good()

bool
StEM_NetMgr::bad()

bool
StEM_NetMgr::fail()

bool
StEM_NetMgr::is_open()

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

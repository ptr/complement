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

using namespace std;
using namespace stem;

class StEM_NetMgr
{
public:
        NetTransportMgr* mgr;

        StEM_NetMgr()
          { this->mgr = new NetTransportMgr; }

        ~StEM_NetMgr()
          { /* delete mgr; */ }

        int open( const char *hostname, int port )
          { return mgr->open( hostname, port ); }

        void close()
          { mgr->close(); }

        int join()
          { return mgr->join(); }
};

// ___________________
// C-Perl
MODULE = stem::NetTransport     PACKAGE = stem::NetTransport

StEM_NetMgr *
StEM_NetMgr::new()

void
StEM_NetMgr::DESTROY()

int
StEM_NetMgr::open( const char *hostname, int port )

void
StEM_NetMgr::close()

int
StEM_NetMgr::join()

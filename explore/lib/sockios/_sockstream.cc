// -*- C++ -*- Time-stamp: <99/02/03 14:53:29 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <sockstream>

#ifdef WIN32

#include <ostream>
#include <xxx/plock.h>

namespace __impl {

static char __xbuff[16];
static Mutex __init_lock;
static const char *WINSOCK_ERR_MSG = "WinSock DLL not 2.0";

} // namespace __impl

namespace std {
int sock_base::Init::__init_cnt = 0;

sock_base::Init::Init()
{
  MT_REENTRANT( __impl::__init_lock, _1 );
  if ( __init_cnt++ == 0 ) {
    WORD    __vers;
    WSADATA __wsadata;

    __vers = MAKEWORD( 2, 0 );
    int __err = WSAStartup( __vers, &__wsadata );

    if ( __err != 0 ) {
      --__init_cnt;
      throw domain_error( __impl::WINSOCK_ERR_MSG );
    }
    if ( LOBYTE(__wsadata.wVersion) != 2 || HIBYTE(__wsadata.wVersion) != 0 ) {
      WSACleanup();
      --__init_cnt;
      throw domain_error( __impl::WINSOCK_ERR_MSG );
    }
  }
}

sock_base::Init::~Init()
{
  MT_REENTRANT( __impl::__init_lock, _1 );
  if ( --__init_cnt == 0 ) {
    WSACleanup();
  }
}

sock_base::sock_base()
{
  new( __impl::__xbuff ) Init();
}

sock_base::~sock_base()
{
  reinterpret_cast<Init *>(__impl::__xbuff)->~Init();
}

} // namespace std

#else // !WIN32

namespace std {

sock_base::sock_base()
{ }

sock_base::~sock_base()
{ }

} // namespace std

#endif // !WIN32

// -*- C++ -*- Time-stamp: <99/05/25 18:50:10 ptr>

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#ifdef WIN32

#include <sockstream>

// The Microsoft's cool programmers made two wanderful things:
// 1. All sockets must be initialized via WSAStartup
// 2. Do it procedure once per every new thread.
// 
// So I do this via Tls* functions (TlsAlloc() in plock.cc)

#include <ostream>

#ifndef __XMT_H
#include <xmt.h>
#endif

extern "C" int APIENTRY
DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
  return TRUE;   // ok
}

namespace __impl {

static char __xbuff[16];
static const char *WINSOCK_ERR_MSG = "WinSock DLL not 2.0";

// extern __declspec( dllexport ) int __thr_key; // xmt.cc
int __thr_key = TlsAlloc();

// static int _sb_idx = Thread::xalloc();

} // namespace __impl

namespace std {

sock_base::Init::Init()
{
  int __tls_init_cnt = (int)TlsGetValue( __impl::__thr_key );
//  long& __tls_init_cnt = __impl::Thread::iword( _sb_idx );
  if ( __tls_init_cnt++ == 0 ) {
    WORD    __vers;
    WSADATA __wsadata;

    __vers = MAKEWORD( 2, 0 );
    int __err = WSAStartup( __vers, &__wsadata );

    if ( __err != 0 ) {
      TlsSetValue( __impl::__thr_key, 0 );
      throw domain_error( __impl::WINSOCK_ERR_MSG );
    }
    if ( LOBYTE(__wsadata.wVersion) != 2 || HIBYTE(__wsadata.wVersion) != 0 ) {
      WSACleanup();
      TlsSetValue( __impl::__thr_key, 0 );
      throw domain_error( __impl::WINSOCK_ERR_MSG );
    }
  }
  TlsSetValue( __impl::__thr_key, (void *)__tls_init_cnt );
}

sock_base::Init::~Init()
{
  int __tls_init_cnt = (int)TlsGetValue( __impl::__thr_key );
  if ( --__tls_init_cnt == 0 ) {
    WSACleanup();
  }
  TlsSetValue( __impl::__thr_key, (void *)__tls_init_cnt );
}

__declspec( dllexport )
sock_base::sock_base()
{
  new( __impl::__xbuff ) Init();
}

__declspec( dllexport )
sock_base::~sock_base()
{
  reinterpret_cast<Init *>(__impl::__xbuff)->~Init();
}

} // namespace std

#endif // WIN32

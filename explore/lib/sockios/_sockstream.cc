// -*- C++ -*- Time-stamp: <99/09/14 15:08:41 ptr>

#ident "$SunId$ %Q%"

#ifdef WIN32

#  ifdef _DLL
#    define __SOCKIOS_DLL __declspec( dllexport )
#  else
#    define __SOCKIOS_DLL
#  endif

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
      OSVERSIONINFO info;
      info.dwOSVersionInfoSize = sizeof(info);
      GetVersionEx(&info);
      
      if ( info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) { // 98 or 95
        if ( info.dwMajorVersion < 4 ) {
          WSACleanup();
          TlsSetValue( __impl::__thr_key, 0 );
          throw domain_error( __impl::WINSOCK_ERR_MSG );          
        }
        if ( info.dwMinorVersion > 0 ) { // that's at least 98, should be socket 2
          WSACleanup();
          TlsSetValue( __impl::__thr_key, 0 );
          throw domain_error( __impl::WINSOCK_ERR_MSG );                    
        }
        // WinSock DLL not 2.0, may be net problems,
        // but this is Win 95...
      } else { // 3.1 or NT, if in NT socket not 2.0 go away.
        WSACleanup();
        TlsSetValue( __impl::__thr_key, 0 );
        throw domain_error( __impl::WINSOCK_ERR_MSG );
      }
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

__SOCKIOS_DLL
sock_base::sock_base()
{
  new( __impl::__xbuff ) Init();
}

__SOCKIOS_DLL
sock_base::~sock_base()
{
  reinterpret_cast<Init *>(__impl::__xbuff)->~Init();
}

} // namespace std

#endif // WIN32

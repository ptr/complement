// -*- C++ -*- Time-stamp: <00/02/24 19:53:29 ptr>

#ident "$SunId$ %Q%"

#ifdef WIN32

#include <sockios/sockstream>

// The Microsoft's cool programmers made two wanderful things:
// 1. All sockets must be initialized via WSAStartup
// 2. Do it procedure once per every new thread.
// 
// So I do this via Tls* functions (TlsAlloc() in plock.cc)

#include <ostream>
#include <sstream>

#ifndef __XMT_H
#include <mt/xmt.h>
#endif

#  if 0
extern "C" int APIENTRY
DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
{
  return TRUE;   // ok
}
#  endif

namespace __impl {

static char __xbuff[16];
static const char *WINSOCK_ERR_MSG  = "WinSock DLL not 2.0";
static const char *WINSOCK_ERR_MSG1 = "WinSock DLL not 2.0 (or not Win95)";

// extern __declspec( dllexport ) int __thr_key; // xmt.cc
int __thr_key = TlsAlloc();

// static int _sb_idx = Thread::xalloc();

} // namespace __impl

namespace std {

static int __glob_init_cnt = 0;
static int __glob_init_wsock2 = 0;

enum {
  WINDOWS_NT_4,
  WINDOWS_NT_3,
  WINDOWS_98,
  WINDOWS_95,
  WINDOWS_95_WSOCK2,
  WINDOWS_3_1
};

int WinVer()
{
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);

  if ( info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) { // Win 9x
    if ( info.dwMinorVersion == 0 ) {
      if ( __glob_init_wsock2 != 0 ) {
        return WINDOWS_95_WSOCK2;
      }
      return WINDOWS_95;
    }
    return WINDOWS_98;
  } else if ( info.dwPlatformId == VER_PLATFORM_WIN32_NT ) { // Win NT
    if ( info.dwMajorVersion == 3 ) {
      return WINDOWS_NT_3;
    }
    return WINDOWS_NT_4;
  }
  return WINDOWS_3_1;
}

__PG_DECLSPEC
sock_base::Init::Init()
{
  int __err = 0;
  try {
    int __tls_init_cnt = (int)TlsGetValue( __impl::__thr_key );
//  long& __tls_init_cnt = __impl::Thread::iword( _sb_idx );
    if ( __tls_init_cnt++ == 0 ) {
      int win_ver = WinVer();

      WORD    __vers;
      WSADATA __wsadata;

      if ( win_ver != WINDOWS_3_1 && !(win_ver == WINDOWS_95 && __glob_init_cnt++ > 0) ) {
        __vers = MAKEWORD( 2, 0 );
        __err = WSAStartup( __vers, &__wsadata );

        if ( __err != 0 && __err != WSAVERNOTSUPPORTED ) {
          TlsSetValue( __impl::__thr_key, 0 );
          throw domain_error( __impl::WINSOCK_ERR_MSG );
        }
        if ( LOBYTE(__wsadata.wVersion) != 2 || HIBYTE(__wsadata.wVersion) != 0 ) {
          if ( win_ver != WINDOWS_95 ) {
            WSACleanup();
            TlsSetValue( __impl::__thr_key, 0 );
            throw domain_error( __impl::WINSOCK_ERR_MSG1 );
          }
        } else if ( win_ver == WINDOWS_95 ) {
          __glob_init_wsock2 = 1;
        }

      }
    }
    TlsSetValue( __impl::__thr_key, (void *)__tls_init_cnt );
  }
  catch ( domain_error& err ) {
    ostringstream ss;
    ss << err.what() << ", Error code " << __err << "\n"
       << __FILE__ << ":" << __LINE__ << endl;
    MessageBox( 0, ss.str().c_str(), "Planet Problem", MB_OK );
    throw;
  }
  catch ( ... ) {
    ostringstream ss;
    ss << "Unspesified exception, Error code " << __err << "\n"
       << __FILE__ << ":" << __LINE__ << endl;
    MessageBox( 0, ss.str().c_str(), "Planet Problem", MB_OK );
    throw;
  }
}

__PG_DECLSPEC
sock_base::Init::~Init()
{
  int __tls_init_cnt = (int)TlsGetValue( __impl::__thr_key );
  int win_ver = WinVer();
  // --__glob_init_cnt; // only for Win 95
  if ( --__tls_init_cnt == 0 ) {
    if ( win_ver != WINDOWS_95 ) {
      WSACleanup();
    } else if ( __glob_init_cnt == 0 ) {
      // WSACleanup();
    }
  }
  TlsSetValue( __impl::__thr_key, (void *)__tls_init_cnt );
}

__PG_DECLSPEC
sock_base::sock_base()
{
  new( __impl::__xbuff ) Init();
}

__PG_DECLSPEC
sock_base::~sock_base()
{
  reinterpret_cast<Init *>(__impl::__xbuff)->~Init();
}

} // namespace std

#endif // WIN32

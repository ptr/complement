#ifndef ___freebsd_dummy_h
#define ___freebsd_dummy_h

#include <config/feature.h>

#ifdef __FIT_NONREENTRANT

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#undef skiprr
#undef ns_initparse
#undef dn_skipname
#undef dn_expand
#undef dn_comp
#undef res_search
#undef res_query
#undef res_querydomain
#undef hostalias
#undef res_init
#undef res_mkquery
#undef res_opt
#undef res_send
#undef res_isourserver
#undef res_nameinquery
#undef res_queriesmatch
#undef res_update
#undef res_dnok
#undef res_dnok
#undef res_mailok
#undef res_ownok
#undef res_hnok
#undef res_send_setqhook
#undef res_send_setrhook
#undef get_nsaddr
#undef res_close
#undef getaddrinfo
#undef freeaddrinfo
#undef gai_strerror
#undef _nsdispatch

#undef __skiprr
#undef __ns_initparse
#undef __dn_skipname
#undef __dn_expand
#undef __dn_comp
#undef __res_search
#undef __res_query
#undef __res_querydomain
#undef __hostalias
#undef __res_init
#undef __res_mkquery
#undef __res_opt
#undef __res_send
#undef __res_isourserver
#undef __res_nameinquery
#undef __res_queriesmatch
#undef __res_update
#undef __res_dnok
#undef __res_dnok
#undef __res_mailok
#undef __res_ownok
#undef __res_hnok
#undef __res_send_setqhook
#undef __res_send_setrhook
#undef __get_nsaddr
#undef __res_close
#undef __getaddrinfo
#undef __freeaddrinfo
#undef __gai_strerror
#undef ___nsdispatch

#define skiprr            x_skiprr
#define ns_initparse      x_ns_initparse
#define dn_skipname       x_dn_skipname
#define dn_expand         x_dn_expand
#define dn_comp           x_dn_comp

#define res_search        x_res_search
#define res_query         x_res_query
#define res_querydomain   x_res_querydomain
#define hostalias         x_hostalias
#define res_init          x_res_init
#define res_mkquery       x_res_mkquery
#define res_opt           x_res_opt
#define res_send          x_res_send
#define res_isourserver   x_res_isourserver
#define res_nameinquery   x_res_nameinquery
#define res_queriesmatch  x_res_queriesmatch
#define res_update        x_res_update
#define res_dnok          x_res_dnok
#define res_mailok        x_res_mailok
#define res_ownok         x_res_ownok
#define res_hnok          x_res_hnok

#define res_send_setqhook x_res_send_setqhook
#define res_send_setrhook x_res_send_setrhook

#define get_nsaddr        x_get_nsaddr

#define res_close         x_res_close

#define getaddrinfo       x_getaddrinfo
#define freeaddrinfo      x_freeaddrinfo

#define gai_strerror      x_gai_strerror

#define _nsdispatch       x__nsdispatch

#define __skiprr            x_skiprr
#define __ns_initparse      x_ns_initparse
#define __dn_skipname       x_dn_skipname
#define __dn_expand         x_dn_expand
#define __dn_comp           x_dn_comp

#define __res_search        x_res_search
#define __res_query         x_res_query
#define __res_querydomain   x_res_querydomain
#define __hostalias         x_hostalias
#define __res_init          x_res_init
#define __res_mkquery       x_res_mkquery
#define __res_opt           x_res_opt
#define __res_send          x_res_send
#define __res_isourserver   x_res_isourserver
#define __res_nameinquery   x_res_nameinquery
#define __res_queriesmatch  x_res_queriesmatch
#define __res_update        x_res_update
#define __res_dnok          x_res_dnok
#define __res_mailok        x_res_mailok
#define __res_ownok         x_res_ownok
#define __res_hnok          x_res_hnok

#define __res_send_setqhook x_res_send_setqhook
#define __res_send_setrhook x_res_send_setrhook

#define __get_nsaddr        x_get_nsaddr

#define __res_close         x_res_close

#define __getaddrinfo       x_getaddrinfo
#define __freeaddrinfo      x_freeaddrinfo

#define __gai_strerror      x_gai_strerror

#define ___nsdispatch       x__nsdispatch
#define __res_send_private  x_res_send_private

int x_res_hnok( const char * );
const char *x_hostalias(const char *);
int x_res_send(const u_char *, int, u_char *, int);
void x_res_close(void);
int x_res_init(void);

#endif /* __FIT_NONREENTRANT */

#endif /* ___freebsd_dummy_h */

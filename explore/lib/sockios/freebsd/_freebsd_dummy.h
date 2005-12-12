#ifndef ___freebsd_dummy_h
#define ___freebsd_dummy_h

#include <config/feature.h>

#ifdef __FIT_NONREENTRANT

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

#endif /* __FIT_NONREENTRANT */

#endif /* ___freebsd_dummy_h */

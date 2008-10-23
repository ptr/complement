/* Time-stamp: <06/09/13 09:42:16 ptr> */

#ifndef __stemdtch_h
#define __stemdtch_h

#ifdef __cplusplus
extern "C" {
#endif

void _send_msg( const char *msg, unsigned msglen );
void _wait_stem();

#ifdef __cplusplus
}
#endif

#endif /* __stemdtch_h */

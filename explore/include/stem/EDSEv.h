// -*- C++ -*- Time-stamp: <01/01/29 13:24:10 ptr>

/*
 *
 * Copyright (c) 1995-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Ltd.
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __EDSEv_h
#define __EDSEv_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

namespace EDS {

#define EV_EDS_CONNECT           0x06
#define EV_EDS_DISCONNECT        0x07
#define EV_EDS_ANNOUNCE          0x08
#define EV_EDS_RQ_SESSION        0x10
#define EV_EDS_RRQ_SESSION       0x11
#define EV_EDS_RS_SESSION        0x12
#define EV_EDS_CL_SESSION        0x13

// Name Service
#define EV_EDS_NM_LIST           0x14
#define EV_EDS_NS_ADDR           0x1c
#define EV_EDS_RQ_ADDR_LIST      0x15
#define EV_EDS_RQ_EXT_ADDR_LIST  0x16
#define EV_EDS_RQ_ADDR_BY_NAME   0x1b

// Cron Service
#define EV_EDS_CRON_ADD          0x17
#define EV_EDS_CRON_REMOVE       0x18
#define EV_EDS_CRON_REMOVE_ARG   0x1d
#define EV_EDS_CRON_START        0x19
#define EV_EDS_CRON_STOP         0x1a
                              // 0x1b
                              // 0x1c
                              // 0x1d

} // namespace EDS

#endif // __EDSEv_h

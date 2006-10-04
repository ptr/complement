// -*- C++ -*- Time-stamp: <06/10/04 09:36:24 ptr>

/*
 * Copyright (c) 1995-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __EDSEv_h
#define __EDSEv_h

namespace stem {

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


#define EV_STEM_NS1_LIST          0x1e
#define EV_STEM_NS1_NAME          0x1f
#define EV_STEM_RQ_ADDR_LIST1     0x20
#define EV_STEM_RQ_EXT_ADDR_LIST1 0x21
#define EV_STEM_RQ_ADDR_BY_NAME1  0x22

// Cron Service
#define EV_EDS_CRON_ADD          0x17
#define EV_EDS_CRON_REMOVE       0x18
#define EV_EDS_CRON_REMOVE_ARG   0x1d
#define EV_EDS_CRON_START        0x19
#define EV_EDS_CRON_STOP         0x1a

} // namespace stem

#endif // __EDSEv_h

// -*- C++ -*- Time-stamp: <06/11/26 14:15:19 ptr>

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

#define EV_STEM_TRANSPORT        0x06
#define EV_STEM_TRANSPORT_ACK    0x07

// #define EV_EDS_CONNECT           0x06
// #define EV_EDS_DISCONNECT        0x07
// #define EV_EDS_ANNOUNCE          0x08
// #define EV_EDS_RQ_SESSION        0x09
// #define EV_EDS_RRQ_SESSION       0x10
// #define EV_EDS_RS_SESSION        0x11
// #define EV_EDS_CL_SESSION        0x12

// Name Service
#define EV_STEM_GET_NS_LIST      0x13
#define EV_STEM_GET_NS_NAME      0x14
#define EV_STEM_NS_LIST          0x15
#define EV_STEM_NS_NAME          0x16

// Cron Service
#define EV_EDS_CRON_ADD          0x17
#define EV_EDS_CRON_REMOVE       0x18
#define EV_EDS_CRON_REMOVE_ARG   0x19
#define EV_EDS_CRON_START        0x1a
#define EV_EDS_CRON_STOP         0x1b

} // namespace stem

#endif // __EDSEv_h

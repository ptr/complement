// -*- C++ -*- Time-stamp: <2011-06-23 17:52:49 yeti>

/*
 * Copyright (c) 1995-1999, 2002-2011
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

#define EV_STEM_SUBSCRIPTION_RQ  0x9
#define EV_STEM_SUBSCRIPTION_FIN 0x10
#define EV_STEM_SUBSCRIPTION     0x11

// remote (net) channel annotation

#define EV_STEM_ANNOTATION       0x12

// Name Service
#define EV_STEM_GET_NS_LIST      0x13
#define EV_STEM_GET_NS_NAME      0x14
#define EV_STEM_NS_LIST          0x15
#define EV_STEM_NS_NAME          0x16

// Cron Service
#define EV_EDS_CRON_ADD          0x17
#define EV_EDS_CRON_REMOVE       0x18
#define EV_EDS_CRON_REMOVE_ARG   0x19

// Maximum StEM special event code:

#define EV_STEM_SYS_MAX          0x30

} // namespace stem

#endif // __EDSEv_h

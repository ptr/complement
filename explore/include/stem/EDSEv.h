// -*- C++ -*- Time-stamp: <99/09/22 10:00:47 ptr>

/*
 *
 * Copyright (c) 1995-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
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

#ident "$SunId$ %Q%"

namespace EDS {

#define EV_EDS_CONNECT           0x06
#define EV_EDS_DISCONNECT        0x07
#define EV_EDS_ANNOUNCE          0x08
#define EV_EDS_RQ_SESSION        0x10
#define EV_EDS_RRQ_SESSION       0x11
#define EV_EDS_RS_SESSION        0x12
#define EV_EDS_CL_SESSION        0x13
#define EV_EDS_NM_LIST           0x14
#define EV_EDS_RQ_ADDR_LIST      0x15
#define EV_EDS_RQ_EXT_ADDR_LIST  0x16

} // namespace EDS

#endif // __EDSEv_h

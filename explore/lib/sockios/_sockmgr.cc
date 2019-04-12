// -*- C++ -*-

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005-2008, 2016, 2019
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <config/feature.h>
#include <cerrno>
#include <sockios/sockstream>

#ifdef STLPORT
_STLP_BEGIN_NAMESPACE
#else
namespace std {
#endif

namespace detail {

using namespace std;

#define BT_HCI_MAX_ACL_SIZE	  (1492 + 4)
#define BT_HCI_MAX_SCO_SIZE	  255
#define BT_HCI_MAX_EVENT_SIZE	260
#define BT_HCI_MAX_FRAME_SIZE (BT_HCI_MAX_ACL_SIZE + 4)

unsigned bt_max_frame = BT_HCI_MAX_FRAME_SIZE;

} // namespace detail

#ifdef STLPORT
_STLP_END_NAMESPACE
#else
} // namespace std
#endif

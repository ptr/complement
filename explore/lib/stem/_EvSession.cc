// -*- C++ -*- Time-stamp: <00/08/01 13:26:52 ptr>

/*
 * Copyright (c) 1995-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics
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

#ident "$SunId$"

#include <EDS/EvSession.h>

namespace EDS {

template class SessionManager<addr_type>;
template class SessionManager<SessionInfo>;

} // namespace EDS

// -*- C++ -*- Time-stamp: <09/06/17 19:07:21 ptr>

/*
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __SOCKIOS_SYSLOG_H
#define __SOCKIOS_SYSLOG_H

#include <sys/syslog.h>
#include <ostream>

namespace misc {

namespace detail {

std::ostream& xsyslog( int, int );

} // namespace detail

inline void open_syslog() {}
void close_syslog();

template <int L, int F>
std::ostream& use_syslog()
{ return detail::xsyslog( L, F ); }

} // namespace misc

#endif // __SOCKIOS_SYSLOG_H

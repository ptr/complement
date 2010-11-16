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

std::ostream& xsyslog( int level, int facility);
std::ostream& xsyslog( int level);
std::ostream& xsyslog();

} // namespace detail

inline void open_syslog() {}
void close_syslog();

void set_default_log_level(int log_level);
void set_default_log_facility(int log_facility);

template <int L, int F>
std::ostream& use_syslog()
{ return detail::xsyslog( L, F ); }

template <int L>
std::ostream& use_syslog()
{ return detail::xsyslog(L); }

std::ostream& use_syslog();

} // namespace misc

#endif // __SOCKIOS_SYSLOG_H

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
#include <string>

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

// converts log level from int to string representation
// returns emtpy string in case of no matches found
std::string convert_log_level(int log_level);

// converts log level from string to int representation
// returns -1 in case of no matches found
int convert_log_level(const std::string& log_level);

// converts log facility from int to string representation
// returns empty string in case of no matches found
std::string convert_log_facility(int log_facility);

// converts log facility from string to int representation
// returns -1 in case of no matches found
int convert_log_facility(const std::string& log_facility);

template <int L, int F>
std::ostream& use_syslog()
{ return detail::xsyslog( L, F ); }

template <int L>
std::ostream& use_syslog()
{ return detail::xsyslog(L); }

std::ostream& use_syslog();

} // namespace misc

#endif // __SOCKIOS_SYSLOG_H

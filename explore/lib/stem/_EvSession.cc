// -*- C++ -*- Time-stamp: <01/03/20 15:47:04 ptr>

/*
 * Copyright (c) 1995-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2001
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

#include <config/feature.h>
#include <EDS/EvSession.h>

namespace EDS {

#ifndef WIN32
__PG_DECLSPEC
SessionInfo::SessionInfo( const SessionInfo& si )
{
  _host = si._host;
  _port = si._port;

  _sz_from = si._sz_from;
  _un_from = si._un_from;
  _sz_to = si._sz_to;
  _un_to = si._un_to;
  _lun_from = si._lun_from;
  _lun_to = si._lun_to;
  _start = si._start;
  _on_line = si._on_line;
  _conn = si._conn;
  _last = si._last;
  _reconnect_cnt = si._reconnect_cnt;
  _is_connected = si._is_connected;

  _control = si._control;
}

__PG_DECLSPEC
SessionInfo& SessionInfo::operator =( const SessionInfo& si )
{
  _host = si._host;
  _port = si._port;

  _sz_from = si._sz_from;
  _un_from = si._un_from;
  _sz_to = si._sz_to;
  _un_to = si._un_to;
  _lun_from = si._lun_from;
  _lun_to = si._lun_to;
  _start = si._start;
  _on_line = si._on_line;
  _conn = si._conn;
  _last = si._last;
  _reconnect_cnt = si._reconnect_cnt;
  _is_connected = si._is_connected;

  _control = si._control;

  return *this;
}
#endif

#if !defined(__HP_aCC) || (__HP_aCC > 1)
template class SessionManager<addr_type>;
template class SessionManager<SessionInfo>;
#endif

} // namespace EDS

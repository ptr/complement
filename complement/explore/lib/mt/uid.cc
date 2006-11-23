// -*- C++ -*- Time-stamp: <06/11/23 17:35:29 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <mt/uid.h>
#include <mt/xmt.h>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace xmt {

namespace detail {

using namespace std;
using namespace xmt;

class __uid_init
{
  public:
    __uid_init();

    static Mutex _lk;
    static uuid_type _host_id;
    static string _host_id_str;
};

Mutex __uid_init::_lk;
uuid_type __uid_init::_host_id;
string __uid_init::_host_id_str;

__uid_init::__uid_init()
{
  Locker lock( _lk );
  ifstream f( "/proc/sys/kernel/random/boot_id" );

  getline( f, _host_id_str );

  istringstream s( _host_id_str );
  char delimiter;

  s >> hex >> _host_id.u.b[0] >> _host_id.u.b[1] >> _host_id.u.b[2] >> _host_id.u.b[3]
    >> delimiter
    >> _host_id.u.b[4] >> _host_id.u.b[5]
    >> delimiter
    >> _host_id.u.b[6] >> _host_id.u.b[7]
    >> delimiter
    >> _host_id.u.b[8] >> _host_id.u.b[9]
    >> delimiter
    >> _host_id.u.b[10] >> _host_id.u.b[11] >> _host_id.u.b[12] >> _host_id.u.b[13] >> _host_id.u.b[14] >> _host_id.u.b[15];
}

} // namespace detail

const std::string& hostid_str()
{
  static detail::__uid_init _uid;
  return detail::__uid_init::_host_id_str;
}

const xmt::uuid_type& hostid()
{
  hostid_str();
  return detail::__uid_init::_host_id;
}

} // namespace xmt

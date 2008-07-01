// -*- C++ -*- Time-stamp: <08/06/06 21:23:34 yeti>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <mt/uid.h>
#include <mt/mutex>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace xmt {

namespace detail {

using namespace std;
using namespace xmt;
using namespace std::tr2;

class __uid_init
{
  public:
    __uid_init();

    static uuid_type _host_id;
    static char _host_id_str[48]; // 37 really
};

uuid_type __uid_init::_host_id;
char __uid_init::_host_id_str[48];

// ifstream _uuid;

__uid_init::__uid_init()
{
  static mutex _lk;

  lock_guard<mutex> lock( _lk );
  ifstream f( "/proc/sys/kernel/random/boot_id" );

  string tmp;
  getline( f, tmp );
  strcpy( _host_id_str, tmp.c_str() );


  stringstream s;
  s << _host_id_str[0]  << _host_id_str[1]  << ' '  
    << _host_id_str[2]  << _host_id_str[3]  << ' '
    << _host_id_str[4]  << _host_id_str[5]  << ' '
    << _host_id_str[6]  << _host_id_str[7]  << ' ' // -
    << _host_id_str[9]  << _host_id_str[10] << ' '
    << _host_id_str[11] << _host_id_str[12] << ' ' // -
    << _host_id_str[14] << _host_id_str[15] << ' '
    << _host_id_str[16] << _host_id_str[17] << ' ' // -
    << _host_id_str[19] << _host_id_str[20] << ' '
    << _host_id_str[21] << _host_id_str[22] << ' ' // -
    << _host_id_str[24] << _host_id_str[25] << ' '
    << _host_id_str[26] << _host_id_str[27] << ' '
    << _host_id_str[28] << _host_id_str[29] << ' '
    << _host_id_str[30] << _host_id_str[31] << ' '
    << _host_id_str[32] << _host_id_str[33] << ' '
    << _host_id_str[34] << _host_id_str[35];
    
  s >> hex
    >> reinterpret_cast<unsigned&>(_host_id.u.b[0])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[1])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[2])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[3])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[4])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[5])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[6])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[7])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[8])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[9])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[10])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[11])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[12])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[13])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[14])
    >> reinterpret_cast<unsigned&>(_host_id.u.b[15]);
}

} // namespace detail

using namespace std;
using namespace std::tr2;

const char *hostid_str()
{
  static detail::__uid_init _uid;
  return detail::__uid_init::_host_id_str;
}

const xmt::uuid_type& hostid()
{
  hostid_str();
  return detail::__uid_init::_host_id;
}

std::string uid_str()
{
  static mutex _lk;

  lock_guard<mutex> lock( _lk );

  // if ( !detail::_uuid.is_open() ) {
  //   detail::_uuid.open( "/proc/sys/kernel/random/uuid" );
  // }

  ifstream _uuid( "/proc/sys/kernel/random/uuid" );

  std::string tmp;

  // getline( detail::_uuid, tmp ).clear(); // clear eof bit
  getline( _uuid, tmp );

  return tmp;
}

xmt::uuid_type uid()
{
  string tmp = uid_str();
  uuid_type id;

  stringstream s;
  s << tmp[0]  << tmp[1]  << ' '  
    << tmp[2]  << tmp[3]  << ' '
    << tmp[4]  << tmp[5]  << ' '
    << tmp[6]  << tmp[7]  << ' ' // -
    << tmp[9]  << tmp[10] << ' '
    << tmp[11] << tmp[12] << ' ' // -
    << tmp[14] << tmp[15] << ' '
    << tmp[16] << tmp[17] << ' ' // -
    << tmp[19] << tmp[20] << ' '
    << tmp[21] << tmp[22] << ' ' // -
    << tmp[24] << tmp[25] << ' '
    << tmp[26] << tmp[27] << ' '
    << tmp[28] << tmp[29] << ' '
    << tmp[30] << tmp[31] << ' '
    << tmp[32] << tmp[33] << ' '
    << tmp[34] << tmp[35];
    
  s >> hex
    >> reinterpret_cast<unsigned&>(id.u.b[0])
    >> reinterpret_cast<unsigned&>(id.u.b[1])
    >> reinterpret_cast<unsigned&>(id.u.b[2])
    >> reinterpret_cast<unsigned&>(id.u.b[3])
    >> reinterpret_cast<unsigned&>(id.u.b[4])
    >> reinterpret_cast<unsigned&>(id.u.b[5])
    >> reinterpret_cast<unsigned&>(id.u.b[6])
    >> reinterpret_cast<unsigned&>(id.u.b[7])
    >> reinterpret_cast<unsigned&>(id.u.b[8])
    >> reinterpret_cast<unsigned&>(id.u.b[9])
    >> reinterpret_cast<unsigned&>(id.u.b[10])
    >> reinterpret_cast<unsigned&>(id.u.b[11])
    >> reinterpret_cast<unsigned&>(id.u.b[12])
    >> reinterpret_cast<unsigned&>(id.u.b[13])
    >> reinterpret_cast<unsigned&>(id.u.b[14])
    >> reinterpret_cast<unsigned&>(id.u.b[15]);

  return id;
}

} // namespace xmt

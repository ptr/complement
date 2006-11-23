// -*- C++ -*- Time-stamp: <06/11/23 16:51:52 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __mt_uid_h
#define __mt_uid_h

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <stdint.h>

namespace xmt {

struct uuid_type
{
  union {
    uint8_t  b[16];
    uint32_t i[4];
  } u;

  
  uuid_type()
  {}

  uuid_type( const uuid_type& uid )
  {
    u.i[0] = uid.u.i[0];
    u.i[1] = uid.u.i[1];
    u.i[2] = uid.u.i[2];
    u.i[3] = uid.u.i[3];
  }


  bool operator ==( const uuid_type& uid )
  { return u.i[0] == uid.u.i[0] && u.i[1] == uid.u.i[1] && u.i[2] == uid.u.i[2] && u.i[3] == uid.u.i[3]; }

  bool operator !=( const uuid_type& uid )
  { return u.i[0] != uid.u.i[0] || u.i[1] != uid.u.i[1] || u.i[2] != uid.u.i[2] || u.i[3] != uid.u.i[3]; }
  
};

const std::string& hostid_str();
const xmt::uuid_type& hostid();

} // namespace xmt

#endif // __mt_uid_h

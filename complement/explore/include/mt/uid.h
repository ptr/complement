// -*- C++ -*- Time-stamp: <06/11/23 23:34:55 ptr>

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
// #include <algorithm>
#include <stdint.h>

namespace xmt {

struct uuid_type
{
  union {
    uint8_t  b[16];
    uint32_t i[4];
    uint64_t l[2];
  } u;

  
  uuid_type()
  {}

  uuid_type( const uuid_type& uid )
  {
    // u.i[0] = uid.u.i[0]; u.i[1] = uid.u.i[1];
    // u.i[2] = uid.u.i[2]; u.i[3] = uid.u.i[3];
    u.l[0] = uid.u.l[0]; u.l[1] = uid.u.l[1];
  }


    bool operator ==( const uuid_type& uid ) const
      {
        // return u.i[0] == uid.u.i[0] && u.i[1] == uid.u.i[1] && u.i[2] == uid.u.i[2] && u.i[3] == uid.u.i[3];
        return u.l[0] == uid.u.l[0] && u.l[1] == uid.u.l[1];
      }
    
    bool operator !=( const uuid_type& uid ) const
      {
        // return u.i[0] != uid.u.i[0] || u.i[1] != uid.u.i[1] || u.i[2] != uid.u.i[2] || u.i[3] != uid.u.i[3];
        return u.l[0] != uid.u.l[0] || u.l[1] != uid.u.l[1];
      }

    bool operator <( const uuid_type& uid ) const
      {
        // return std::lexicographical_compare( u.i, u.i + 4, uid.u.i, uid.u.i + 4 );
        return u.l < uid.u.l ? true : u.l > uid.u.l ? false : (u.l[1] < uid.u.l[1]);
      }
};

const std::string& hostid_str();
const xmt::uuid_type& hostid();

} // namespace xmt

#endif // __mt_uid_h

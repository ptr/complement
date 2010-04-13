// -*- C++ -*- Time-stamp: <10/04/13 17:42:39 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __yard_yard_h
#define __yard_yard_h

#include <fstream>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <mt/uid.h>


namespace yard {

typedef xmt::uuid_type id_type;

class yard
{
  public:
    typedef size_t size_type;
    typedef off_t  offset_type;

    yard( const char* path );
    ~yard();

    id_type put_revision( const void*, size_type ) throw (std::ios_base::failure);
    id_type put_revision( const std::string& s ) throw (std::ios_base::failure)
      { return put_revision( s.data(), s.length() ); }

    std::string get( const id_type& ) throw (std::ios_base::failure, std::invalid_argument);

  private:
    offset_type create_block( int ) throw (std::ios_base::failure);
    id_type put_raw( const id_type&, const void*, size_type ) throw (std::ios_base::failure);

  private:
    static const size_t first_hash_size;
    static const size_t block_size;
    static const size_t hash_block_n;
    static const size_t hash_block_id_off;
    static const size_t hash_block_off_off;

    std::fstream f;
    offset_type* block_offset;
};

} // namespace yard

#endif // __yard_yard_h

// -*- C++ -*- Time-stamp: <10/04/16 09:47:48 ptr>

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

class underground
{
  public:
    typedef size_t size_type;
    typedef off_t  offset_type;

    underground( const char* path );
    ~underground();

    id_type put_revision( const void*, size_type ) throw (std::ios_base::failure);
    id_type put_revision( const std::string& s ) throw (std::ios_base::failure)
      { return put_revision( s.data(), s.length() ); }

    void put_object( const id_type&, const void*, size_type ) throw (std::ios_base::failure);
    void put_object( const id_type& id, const std::string& s ) throw (std::ios_base::failure)
      { put_object( id, s.data(), s.length() ); }

    std::string get( const id_type& ) throw (std::ios_base::failure, std::invalid_argument);
    std::string get_object( const id_type& ) throw (std::ios_base::failure, std::invalid_argument);

  private:
    offset_type create_block( int ) throw (std::ios_base::failure);
    void put_raw( const id_type&, const void*, size_type ) throw (std::ios_base::failure);
    offset_type get_priv( const id_type&, std::string& ) throw (std::ios_base::failure, std::invalid_argument);

  private:
    static const size_t first_hash_size;
    static const size_t block_size;
    static const size_t hash_block_n;
    static const size_t hash_block_id_off;
    static const size_t hash_block_off_off;

    std::fstream f;
    offset_type* block_offset;
};

class yard
{
  public:
    yard( const char* path );
    ~yard();

  private:
    struct vertex
    {
        int type;
        id_type id;
        id_type rid;
        bool mod_flag;
    };

    underground* disc;
};

} // namespace yard

#endif // __yard_yard_h

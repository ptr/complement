// -*- C++ -*- Time-stamp: <10/05/12 14:50:49 ptr>

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
#include <mt/uidhash.h>

#include <list>

#ifdef STLPORT
#  include <unordered_map>
#  define __USE_STLPORT_TR1
#else
#  if defined(__GNUC__) && (__GNUC__ < 4)
#    include <ext/hash_map>
#    define __USE_STD_HASH
#  else
#    include <tr1/unordered_map>
#    define __USE_STD_TR1
#  endif
#endif

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

    id_type put_object( const id_type&, const void*, size_type ) throw (std::ios_base::failure);
    id_type put_object( const id_type& id, const std::string& s ) throw (std::ios_base::failure)
      { return put_object( id, s.data(), s.length() ); }

    std::string get( const id_type& ) throw (std::ios_base::failure, std::invalid_argument);
    std::string get_object( const id_type& ) throw (std::ios_base::failure, std::invalid_argument);
    std::pair<std::string,id_type> get_object_r( const id_type& ) throw (std::ios_base::failure, std::invalid_argument);

    void flush();

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
    typedef size_t size_type;

    yard( const char* path );
    ~yard();

    void add_manifest( const id_type& );
    void add_manifest( const id_type&, const id_type& );
    void add_leaf( const id_type&, const void*, size_type );
    void add_leaf( const id_type& id, const std::string& s )
      { yard::add_leaf( id, s.data(), s.size() ); }
    void add_leaf( const id_type&, const id_type&, const void*, size_type );
    void add_leaf( const id_type& mid, const id_type& id, const std::string& s )
      { yard::add_leaf( mid, id, s.data(), s.size() ); }

    std::string get( const id_type& );

    void flush();

  private:

    enum {
      unknown = 0,
      manifest = 1,
      leaf = 2
    };

    struct vertex
    {
        typedef vertex* pointer_type;
        typedef std::list<std::pair<pointer_type,uint32_t> > adj_list_type;

        int type;
        id_type id;
        id_type rid;
        std::string blob;
        bool mod_flag;
        adj_list_type adj_list;
    };

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<id_type,vertex::pointer_type> graph_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<id_type,vertex::pointer_type> graph_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<id_type,vertex::pointer_type> graph_type;
#endif

    graph_type g;
    underground* disc;
};

#ifdef __USE_STLPORT_HASH
#  undef __USE_STLPORT_HASH
#endif
#ifdef __USE_STD_HASH
#  undef __USE_STD_HASH
#endif
#ifdef __USE_STLPORT_TR1
#  undef __USE_STLPORT_TR1
#endif
#ifdef __USE_STD_TR1
#  undef __USE_STD_TR1
#endif

} // namespace yard

#endif // __yard_yard_h

// -*- C++ -*- Time-stamp: <2011-02-02 19:10:50 ptr>

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

#include <vector>
#include <memory>
#include <map>
#include <set>
#include <stack>

namespace yard {

typedef xmt::uuid_type revision_id_type;
typedef revision_id_type manifest_id_type;
typedef xmt::uuid_type commit_id_type;
typedef xmt::uuid_type meta_id_type;
typedef std::map<std::string,revision_id_type> manifest_type;
typedef std::pair<manifest_type,manifest_type> diff_type;
typedef std::list<std::pair<std::string,std::pair<revision_id_type,revision_id_type> > > conflicts_list_type;

struct commit_node
{
    manifest_id_type mid;
    diff_type* delta;
    int dref;
    enum {
      white = 0,
      gray,
      black
    } color;

    typedef std::list<commit_id_type> edge_container_type;
    edge_container_type edge_in;
};

class metainfo
{
  public:
    void set( int, const std::string& );
    bool is_set( int );
    const std::string& get( int );

  private:
    typedef std::pair<uint8_t,std::string> record_type;
    typedef std::list<record_type> container_type;

    container_type rec;
};

struct header_type
{
    uint32_t version;
    uint32_t block_size;
    uint32_t address_of_the_root;

    void pack(std::ostream& s) const;
    void unpack(std::istream& s);
};

class BTree
{
  public:
    typedef std::streambuf::off_type off_type;
    typedef xmt::uuid_type key_type;

  public:
    struct block_coordinate
    {
        off_type address;
        std::streamsize size;
    };

#ifndef __FIT_YARD_UT
  private:
#else
  public:
#endif
    class block_type
    {
      private:
        typedef std::map<key_type, block_coordinate> body_type;

      public:
        typedef body_type::const_iterator const_iterator;
        typedef body_type::iterator iterator;

        enum {
          root_node = 1,
          leaf_node = 2
        };

        bool is_root() const;
        bool is_leaf() const;

        void set_flags(unsigned int flags);

        const_iterator find(const key_type& key) const;

        bool insert(const key_type& key, const block_coordinate& coordinate);
        int erase(const key_type& key);

        const_iterator begin() const;
        const_iterator end() const;

        const_iterator route(const key_type& key) const;

        bool is_overfilled() const;

        void divide(block_type& other);

        void pack(std::ostream& s) const;
        void unpack(std::istream& s);

        void set_block_size( std::streamsize block_size );
        std::streamsize get_block_size() const;

        block_type();

      private:
        void calculate_size();

        key_type min() const;
        key_type max() const;

        std::streamsize block_size_;
        unsigned flags_;
        std::streamsize size_of_packed_;

        body_type body_;
    };

  public:
    typedef std::pair<off_type, std::pair<key_type, key_type> > block_desc;
    typedef std::stack<block_desc> coordinate_type;

    coordinate_type lookup(const key_type& key);
    coordinate_type lookup(const coordinate_type& start, const key_type& key);
    const block_type& get(const coordinate_type& coordinate);
    coordinate_type insert(coordinate_type path, const key_type& key, const block_coordinate& coord);

    void open( const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, uint32_t block_size = 4096);
    void close();

    off_type add_value( const char* data, std::streamsize size );
    void clear_cache();

  private:
    block_type& get_block(off_type offset);

    void lookup_down(coordinate_type& path, const key_type& key);
    key_type get_shortest_key(const key_type& first, const key_type& second);

    off_type append(const block_type& block);
    void save(off_type offset, const block_type& block);
    void load(off_type offset, block_type& block);

    key_type min_in_subtree( off_type block_address );
    key_type max_in_subtree( off_type block_address );

    std::fstream file_;
    std::map<off_type, block_type> cache_;
    header_type header_;
};

struct revision_node
{
    enum 
    {
      mod = 0x1,
      meta_loaded = 0x2
    };

    int flags;
    std::string content;
    meta_id_type mid;
};

class revision
{
  public:
    revision_id_type push( const void*, size_t );
    revision_id_type push( const std::string& data )
      { return revision::push( data.data(), data.length() ); }
    revision_id_type push( const manifest_type& );
    revision_id_type push( const diff_type& );
    revision_id_type push( const commit_node&, const commit_id_type& );
    const std::string& get( const revision_id_type& ) throw( std::invalid_argument );

    void get_manifest( manifest_type&, const revision_id_type& ) throw( std::invalid_argument );
    void get_diff( diff_type&, const revision_id_type& ) throw( std::invalid_argument );
    void get_commit( commit_node&, const revision_id_type& ) throw( std::invalid_argument );

  private:
    typedef std::map<revision_id_type,revision_node> revisions_container_type;

    revisions_container_type r;
};

class yard
{
  public:
    yard();

    /* commit id generated on client side instead of within yard_ng,
       because of potential distributed nature of db (id should be same
       on every node of group).
     */
    void open_commit_delta( const commit_id_type& base, const commit_id_type& id );
    void close_commit_delta( const commit_id_type& id );

    void add( const commit_id_type&, const std::string&, const void*, size_t );
    void add( const commit_id_type& id, const std::string& name, const std::string& data )
      { add( id, name, data.data(), data.length() ); }
    void del( const commit_id_type&, const std::string& );

    const std::string& get( const commit_id_type&, const std::string& ) throw( std::invalid_argument, std::logic_error );
    const std::string& get( const std::string& ) throw( std::invalid_argument, std::logic_error );
    const std::string& get( const revision_id_type& id ) throw( std::invalid_argument )
      { return r.get( id ); }

    diff_type diff( const commit_id_type&, const commit_id_type& );

    /* again, commit id generated on client side instead of within yard_ng,
       because of potential distributed nature of db (id should be same
       on every node of group).
     */
    int fast_merge( const commit_id_type& merge, const commit_id_type& left, const commit_id_type& right );
    int merge( const commit_id_type& merge, const commit_id_type& left, const commit_id_type& right, conflicts_list_type& );

    template <class BackInsertIterator>
    void heads( BackInsertIterator bi )
      { std::copy( leaf.begin(), leaf.end(), bi ); }

  private:
    typedef std::map<commit_id_type,commit_node> commit_container_type;
    typedef std::list<commit_id_type> leafs_container_type;
    typedef std::map<commit_id_type,std::pair<std::pair<commit_id_type,commit_id_type>,diff_type> > cache_container_type;
    typedef std::map<manifest_id_type,manifest_type> cached_manifest_type;
    typedef std::map<meta_id_type,std::pair<bool,metainfo> > meta_container_type;

    commit_id_type common_ancestor( const commit_id_type&, const commit_id_type& );
    manifest_id_type aggregate_delta( const commit_id_type&, diff_type& );

    revision r;
    commit_container_type c;
    leafs_container_type leaf;
    cache_container_type cache;
    cached_manifest_type cached_manifest;
    meta_container_type meta;
};

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

    template <class BackInsertIterator>
    void get_object_revisions( const id_type& id, BackInsertIterator bi ) throw (std::ios_base::failure, std::invalid_argument)
      {
        std::string rev;
        get_priv( id, rev ); // object's revisions

        // extract revisions
        if ( rev.size() >= sizeof(id_type) ) {
          for ( const char* p = rev.data(); p < (rev.data() + rev.size()); p += sizeof(id_type) ) {
            *bi++ = *reinterpret_cast<const id_type*>(p);
          }
        }
      }

    void flush();

  private:
    offset_type create_block( int ) throw (std::ios_base::failure);
    void put_raw( const id_type&, const void*, size_type ) throw (std::ios_base::failure);
    offset_type get_priv( const id_type&, std::string& ) throw (std::ios_base::failure, std::invalid_argument);

  private:
    static const size_t block_size;
    static const size_t hash_block_n;
    static const size_t hash_block_id_off;
    static const size_t hash_block_off_off;

#ifdef __USE_STLPORT_HASH
    typedef std::hash_map<id_type,offset_type> hash_table_type;
#endif
#ifdef __USE_STD_HASH
    typedef __gnu_cxx::hash_map<id_type,offset_type> hash_table_type;
#endif
#if defined(__USE_STLPORT_TR1) || defined(__USE_STD_TR1)
    typedef std::tr1::unordered_map<id_type,offset_type> hash_table_type;
#endif

    std::fstream f;
    offset_type hoff;
    offset_type ds; // data section offset
    size_type hsz;  // stored hash baskets size
    offset_type* block_offset;
    // hash_table_type cache; // objects offset cache
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

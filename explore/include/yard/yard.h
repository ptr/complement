// -*- C++ -*- Time-stamp: <2011-02-15 14:22:35 ptr>

/*
 *
 * Copyright (c) 2010-2011
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

namespace detail {

struct block_coordinate
{
    typedef std::streambuf::off_type off_type;

    off_type address;
    std::streamsize size;
};

class block_type
{
  public:
    typedef xmt::uuid_type key_type;

  private:
    typedef std::map<key_type, block_coordinate> body_type;

  public:
    typedef body_type::const_iterator const_iterator;
    typedef body_type::iterator iterator;

    enum {
      root_node = 1,
      leaf_node = 2
    };

    block_type();

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

  private:
    std::streamsize size_of_packed_entry(const_iterator it);
    void calculate_size();

    key_type min() const;
    key_type max() const;

    std::streamsize block_size_;
    unsigned flags_;
    std::streamsize size_of_packed_;

    body_type body_;
};

} // namespace detail

class BTree
{
  public:
    typedef std::streambuf::off_type off_type;
    typedef xmt::uuid_type key_type;

  private:
    enum {
      magic = 0,      // magic number
      cgleaf_off = 1, // offset of commits ids graph leafs list
      ver = 2,        // version of file format
      defbsz = 3,     // default block size
      rb_off = 4,     // offset of root
      toc_reserve = 5 // reserved space for toc
    };

  public:

  public:
    typedef std::pair<off_type, std::pair<key_type, key_type> > block_desc;
    typedef std::stack<block_desc> coordinate_type;

    BTree();
    BTree( const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, std::streamsize block_size = 4096 );
    ~BTree();

    bool is_open();
    bool good() const;
    bool bad() const;

    void flush();

    coordinate_type lookup(const key_type& key);
    coordinate_type lookup(const coordinate_type& start, const key_type& key);
    const detail::block_type& get(const coordinate_type& coordinate);
    coordinate_type insert(coordinate_type path, const key_type& key, const detail::block_coordinate& coord);

    void open( const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, std::streamsize block_size = 4096);
    void close();

    off_type add_value( const char* data, std::streamsize size );
    void clear_cache();

  private:
    detail::block_type& get_block(off_type offset);

    void lookup_down(coordinate_type& path, const key_type& key);
    key_type get_shortest_key(const key_type& first, const key_type& second);

    off_type append(const detail::block_type& block);
    void save(off_type offset, const detail::block_type& block);
    void load(off_type offset, detail::block_type& block);

    key_type min_in_subtree( off_type block_address );
    key_type max_in_subtree( off_type block_address );

    std::fstream file_;
    std::map<off_type, detail::block_type> cache_;

    std::streamsize bsz; // default block size
    off_type root_block_off;
    int format_ver;
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
    revision();
    revision( const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, std::streamsize block_size = 4096 );
    ~revision();

    void open( const char* filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, std::streamsize block_size = 4096);

    bool is_open()
      { return db.is_open(); }
    bool good() const
      { return db.good(); }
    bool bad() const
      { return db.bad(); }

    void flush();

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
    BTree db;
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

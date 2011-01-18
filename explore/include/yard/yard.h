// -*- C++ -*- Time-stamp: <2010-12-20 13:13:08 ptr>

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

#include <yard/iterator.h>
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

typedef unsigned int file_address_type;

file_address_type seek_to_end(std::fstream& file, unsigned int size);

struct index_node_entry
{
    xmt::uuid_type key;
    file_address_type pointer;

    xmt::uuid_type get_key() const
    {
        return key;
    }

    file_address_type get_pointer() const
    {
        return pointer;
    }

    bool operator>(const index_node_entry& right) const
    {
        return (key > right.key);
    }
};

struct data_node_entry
{
    xmt::uuid_type key;
    file_address_type address_of_value;
    unsigned int size;

    xmt::uuid_type get_key() const
    {
        return key;
    }

    file_address_type get_address_of_value() const
    {
        return address_of_value;
    }

    unsigned int get_size() const
    {
        return size;
    }

    bool operator>(const data_node_entry& right) const
    {
        return (key > right.key);
    }
};

class block_type
{
public:
    static const unsigned int root_node;
    static const unsigned int leaf_node;

    template <typename T>
    class key_readable : public std::iterator<std::input_iterator_tag, xmt::uuid_type>
    {
    protected:
        T pointer_;
        unsigned int type_;
    public:
        enum
        {
            index = 0,
            data = 1
        };

        key_readable(T pointer, unsigned int type):
            pointer_(pointer),
            type_(type)
        {}

        T get() const
        {
            return pointer_;
        }

        xmt::uuid_type operator*()
        {
            if (type_ == 0)
                return ((const index_node_entry*)pointer_)->get_key();
            else
                return ((const data_node_entry*)pointer_)->get_key();
        }
    };

    typedef array_iterator<key_readable<const char*> > const_key_iterator;
    typedef array_iterator<key_readable<char*> > key_iterator;

    typedef data_node_entry* data_iterator;
    typedef index_node_entry* index_iterator;
    typedef const data_node_entry* data_const_iterator;
    typedef const index_node_entry* index_const_iterator;

    bool is_root() const;
    bool is_leaf() const;

    const_key_iterator key_begin() const;
    const_key_iterator key_end() const;
    key_iterator key_begin();
    key_iterator key_end();

    template <typename T>
    const T* get_entry(const const_key_iterator& coordinate) const
    {
        return (const T*)(coordinate.get());
    }

    template <typename T>
    T* get_entry(const key_iterator& coordinate)
    {
        return (T*)(coordinate.get());
    }

    void set_flags(unsigned int flags);

    void insert_index(const index_node_entry& entry);
    void insert_data(const data_node_entry& entry);

    data_const_iterator lookup(xmt::uuid_type key) const;
    index_const_iterator route(xmt::uuid_type key) const;

    bool is_overfilled() const;

    std::pair<xmt::uuid_type, xmt::uuid_type> divide(block_type& other);

    void pack(std::ostream& s) const;
    void unpack(std::istream& s);

    static unsigned int disk_block_size();

    block_type();
private:
    static const unsigned int index_node_nsize;
    static const unsigned int data_node_nsize;

    xmt::uuid_type min() const;
    xmt::uuid_type max() const;

    unsigned int flags_;
    unsigned int size_;
    char data_[4096 - 2*sizeof(unsigned int)];
};

class BTree
{
public:
    typedef std::stack<file_address_type> coordinate_type;
    coordinate_type lookup(xmt::uuid_type key);
    const block_type& get(const coordinate_type& coordinate);
    void insert(coordinate_type path, const data_node_entry& data);

    void init_empty(const char* filename);
    void init_existed(const char* filename);

    file_address_type add_value(const char* data, unsigned int size);
    void clear_cache();
private:
    void lookup(coordinate_type& path, xmt::uuid_type key);
    void insert(coordinate_type path, const index_node_entry& data);

    std::fstream file_;
    std::map<file_address_type, block_type> cache_;
    file_address_type root_address_;
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
    const std::string& get( const revision_id_type& ) throw( std::invalid_argument );

  private:
    typedef std::map<revision_id_type,revision_node> revisions_container_type;

    revisions_container_type r;
};

class yard_ng
{
  public:
    yard_ng();

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
    struct commit_node
    {
        manifest_id_type mid;
        enum {
          white = 0,
          gray,
          black
        } color;

        typedef std::list<commit_id_type> edge_container_type;
        edge_container_type edge_in;
        edge_container_type edge_out;
    };

    typedef std::map<commit_id_type,commit_node> commit_container_type;
    typedef std::list<commit_id_type> leafs_container_type;
    typedef std::map<commit_id_type,std::pair<std::pair<commit_id_type,commit_id_type>,manifest_type> > cache_container_type;
    typedef std::map<manifest_id_type,manifest_type> cached_manifest_type;
    typedef std::map<meta_id_type,std::pair<bool,metainfo> > meta_container_type;

    commit_id_type common_ancestor( const commit_id_type&, const commit_id_type& );

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

class yard
{
  public:
    typedef size_t size_type;
    typedef off_t offset_type;

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

    template <class BackInsertIterator>
    void get_revisions( const id_type& id, BackInsertIterator bi )
      { disc->get_object_revisions( id, bi ); }

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
    // typedef std::tr1::unordered_map<id_type,offset_type> hash_table_type;
#endif

    graph_type g;
    underground* disc;
    // hash_table_type ht;
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

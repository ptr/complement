// -*- C++ -*- Time-stamp: <99/12/01 19:50:47 ptr>

#ifndef __SQL_h
#define __SQL_h

#ident "$SunId$ %Q%"

#if defined( __DB_POSTGRES )
extern "C" {
  struct pg_conn;
} // extern "C"
#elif defined( __DB_MYSQL )
extern "C" {
  struct st_mysql;
} // extern "C"
#else
#  error "------------ Unsupported DB -------------"
#endif

#ifndef __config_feature_h
#include <config/feature.h>
#endif

#include <string>
#include <vector>
#include <utility>
#include <sstream>

namespace database {

#if defined( __DB_POSTGRES )
typedef ::pg_conn    DBconn;
#elif defined( __DB_MYSQL )
typedef ::st_mysql  DBconn;
#endif

class DataBase;

struct NILL_type
{
};

extern NILL_type NILL;

class Table
{
  protected:
    enum _data_type {
      _NILL,
      _NONNILL
    };

    typedef std::pair<std::string,_data_type> entry_type;

  public:
    Table( const char *n, DataBase *_db ) :
        _name( n ),
        db( _db )
      { }
    Table( const std::string& n, DataBase *_db ) :
        _name( n ),
        db( _db )
      { }

    Table& INSERT()
      {
        _op = _INSERT;
        _count = 0;
        for ( int i = 0; i < header.size(); ++i ) {
          header[i].second = _NILL;
        }
        return *this;
      }

    Table& operator <<( const struct NILL_type& )
      {
        header[_count++].second = _NILL;
        return *this;
      }

    Table& operator <<( int x )
      {
        std::ostringstream s;
        s << x;
        header[_count].second = _NONNILL;
        row[_count++] = s.str();
        return *this;
      }

    Table& operator <<( unsigned x )
      {
        std::ostringstream s;
        s << x;
        header[_count].second = _NONNILL;
        row[_count++] = s.str();
        return *this;
      }

    Table& operator <<( long x )
      {
        std::ostringstream s;
        s << x;
        header[_count].second = _NONNILL;
        row[_count++] = s.str();
        return *this;
      }

    Table& operator <<( unsigned long x )
      {
        std::ostringstream s;
        s << x;
        header[_count].second = _NONNILL;
        row[_count++] = s.str();
        return *this;
      }

    Table& operator <<( const char *x )
      {
        header[_count].second = _NONNILL;
        row[_count] = "'";
        row[_count] += x;
        std::string::size_type p = row[_count].rfind( '\'' );
        
        while ( p != 0 ) {
          row[_count].insert( p, 1, '\\' );
          p = row[_count].rfind( '\'', p );
        }
        row[_count++] += "'";
        return *this;
      }

     Table& operator <<( const std::string& x )
      {
        header[_count].second = _NONNILL;
        row[_count] = "'";
        row[_count] += x;
        std::string::size_type p = row[_count].rfind( '\'' );
        
        while ( p != 0 ) {
          row[_count].insert( p, 1, '\\' );
          p = row[_count].rfind( '\'', p );
        }
        row[_count++] += "'";
        return *this;
      }

    void done();
    const std::string& name()
      { return _name; }

    // protected:

    void field( const char *fn )
      {
        std::string s;
        header.push_back( entry_type(fn,_NILL) );
        row.push_back( s );
      }

  protected:
    std::string _name;
    std::vector<entry_type>  header;
    std::vector<std::string> row;
    int _count;
    DataBase *db;

    enum _op_type {
      _NONE,
      _INSERT
    } _op;
};

class DataBase
{
  public:
    enum {
      goodbit = 0,
      badbit  = 1,
      failbit = 2
    };

    DataBase( const char *name, const char *usr = 0, const char *passwd = 0,
              const char *host = 0, const char *port = 0, const char *opt = 0,
              const char *tty = 0 );
    ~DataBase();

    bool good() const
      { return _flags == goodbit; }
    bool bad() const
      { return (_flags & badbit) != 0; }
    bool fail() const
      { return (_flags & failbit) != 0; }
    void clear()
      { _flags = goodbit; }
    void exec( const std::string& );

    void exec();

    void state( const std::string& s )
      {
        if ( sentence.length() != 0 ) {
          sentence += "; ";
        }
        sentence += s;
      }

    Table& table( const char * );
    void begin_transaction();
    void end_transaction();

  protected:
    Table& define_table( const char * );

  private:
    // std::string _exec_str;
    DBconn *_conn;
    unsigned _flags;
    typedef std::vector<Table*> tbl_container_type;
    tbl_container_type tables;

    std::string sentence;

    friend class Transaction;
    friend class Cursor;
};

class Transaction
{
  public:
    Transaction( DataBase& );
    ~Transaction();

    void exec( const std::string& s )
      { _exec_str = s; }

  private:
    DataBase& _db;
    std::string _exec_str;
    friend class Cursor;
};

class Cursor
{
  public:
    Cursor( Transaction&, const char *nm, const char *select );
    ~Cursor();


    void fetch_all();
    const std::string& value( int, const std::string& );
    std::vector<std::vector<std::string> >::size_type size() const
      { return data.size(); }

  private:
    std::string name;
    Transaction& tr;
    std::vector<std::string> fields;
    std::vector<std::vector<std::string> > data;
};

#if 0
class cursor_iterator :
    public std::iterator<std::input_iterator_tag,std::string,
                         std::ptrdiff_t,std::string*,std::string&>
{
  public:
    typedef std::vector<<std::string>      vector_type;
    typedef Cursor                         cursor_type;
    typedef int                            int_type;

    // class to maintain a character and an associated streambuf
    class proxy {
	vector_type  __keep;
	cursor_type *__cursor;

	proxy( cursor_type *c ) throw() :
	    __cursor(c)
	  { }
	proxy( vector_type v, cursor_type *c ) throw() :
	    __keep(v), __cursor(c)
	  { }

      public:

	vector_type& operator*() throw()
	  { return __keep; }

	friend class cursor_iterator;
    };

  public:
    cursor_iterator() throw() :
	__p(0)
      { __failed_flag = true; }
//    cursor_iterator( istream_type& s ) throw() :
//	__sbuf( s.rdbuf() )
//      { __failed_flag = __sbuf ? false : true; }
    cursor_iterator( cursor_type *c ) throw() :
	__p(c)
      { __failed_flag = __p.__cursor ? false : true; }
    cursor_iterator( const proxy& p ) throw() :
	__p( p.__cursor )
      { }
    vector_type& operator*()
      {
	if ( __cursor && !__failed_flag ) {
          // FETCH FORWARD 1 FROM __p.__cursor->name
//	  int_type c = __sbuf->sgetc();
//	  if ( traits::eq_int_type(c,traits::eof()) ) {
//	    __sbuf = 0;
//	    __failed_flag = true;
//	  } else {
//            return traits::to_char_type(c);
//          }
	}
        return __eoc;
      }
    cursor_iterator& operator++()
      {
	if ( __p.__cursor && !__failed_flag && true
             /* FETCH FORWARD 1 FROM __p.__cursor->name */
              ) { 
          __p.__cursor = 0;
          __failed_flag = true;
	}
	return *this;
      }
    proxy operator++(int)
      {
	if ( __p.__cursor && !__failed_flag ) {
//	  proxy prev(__sbuf->sgetc(), __sbuf);
//	  if ( traits::eq_int_type(__sbuf->snextc(),traits::eof()) ) {
//	    __sbuf = 0;
//	    __failed_flag = true;
//	  }  
//	  return prev;
	}
    
	return proxy( __p.__cursor );
      }
    bool equal( cursor_iterator>& c )
      {
	if ( ((__p.__cursor == 0 ) && ( c.__p.__cursor == 0 )) ||
	     (__p.__cursor->name == c.__p.__cursor->name ) )
	  return true;
	else
	  return false;
      }

    bool failed( ) const throw()
    { return __failed_flag; }

  private:
    proxy __p;
    bool  __failed_flag;
    static vector_type __eof;
};

#endif


} // namespace database

#endif // __SQL_h

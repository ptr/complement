// -*- C++ -*- Time-stamp: <99/08/27 15:19:06 ptr>

#ifndef __SQL_h
#define __SQL_h

#ident "$SunId$ %Q%"

extern "C" {
struct pg_conn;
// struct PGconn;
} // extern "C"

#include <string>
#include <vector>

namespace database {

class DataBase
{
  public:
    enum {
      goodbit = 0,
      badbit  = 1,
      failbit = 2
    };

    DataBase( const char *name,
              const char *host = 0, const char *port = 0, const char *opt = 0,
              const char *tty = 0 );
    ~DataBase();

    bool good() const
      { return _flags == goodbit; }
    bool bad() const
      { return (_flags & badbit) != 0; }
    bool fail() const
      { return (_flags & failbit) != 0; }

  private:
    // std::string _exec_str;
    ::pg_conn *_conn;
    unsigned _flags;

    void exec( const std::string& );
    friend class Transaction;
    friend class Cursor;
};

class Transaction
{
  public:
    Transaction( DataBase& );
    ~Transaction();

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

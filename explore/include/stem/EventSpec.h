// -*- C++ -*- Time-stamp: <99/04/01 14:41:35 ptr>

// Only for MS VC!
// don't direct include!

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

class Event_base<short> :
        public __Event_Base
{
  public:
    typedef short         value_type;
    typedef short&        reference;
    typedef const short&  const_reference;
    typedef short *       pointer;
    typedef const short * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const short& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const short& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

class Event_base<unsigned short> :
        public __Event_Base
{
  public:
    typedef unsigned short         value_type;
    typedef unsigned short&        reference;
    typedef const unsigned short&  const_reference;
    typedef unsigned short *       pointer;
    typedef const unsigned short * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const unsigned short& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const unsigned short& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

class Event_base<int> :
        public __Event_Base
{
  public:
    typedef int         value_type;
    typedef int&        reference;
    typedef const int&  const_reference;
    typedef int *       pointer;
    typedef const int * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const int& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const int& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

class Event_base<unsigned> :
        public __Event_Base
{
  public:
    typedef unsigned         value_type;
    typedef unsigned&        reference;
    typedef const unsigned&  const_reference;
    typedef unsigned *       pointer;
    typedef const unsigned * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const unsigned& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const unsigned& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

class Event_base<long> :
        public __Event_Base
{
  public:
    typedef long         value_type;
    typedef long&        reference;
    typedef const long&  const_reference;
    typedef long *       pointer;
    typedef const long * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const long& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const long& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

class Event_base<unsigned long> :
        public __Event_Base
{
  public:
    typedef unsigned long         value_type;
    typedef unsigned long&        reference;
    typedef const unsigned long&  const_reference;
    typedef unsigned long *       pointer;
    typedef const unsigned long * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const unsigned long& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const unsigned long& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

class Event_base<char> :
        public __Event_Base
{
  public:
    typedef char         value_type;
    typedef char&        reference;
    typedef const char&  const_reference;
    typedef char *       pointer;
    typedef const char * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const char& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const char& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

class Event_base<unsigned char> :
        public __Event_Base
{
  public:
    typedef unsigned char         value_type;
    typedef unsigned char&        reference;
    typedef const unsigned char&  const_reference;
    typedef unsigned char *       pointer;
    typedef const unsigned char * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const unsigned char& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const unsigned char& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

class Event_base<signed char> :
        public __Event_Base
{
  public:
    typedef signed char         value_type;
    typedef signed char&        reference;
    typedef const signed char&  const_reference;
    typedef signed char *       pointer;
    typedef const signed char * const_pointer;

    Event_base() :
        __Event_Base(),
        _data()
      { }

    explicit Event_base( code_type c ) :
        __Event_Base( c ),
        _data()
      { }

    explicit Event_base( code_type c, const signed char& d ) :
        __Event_Base( c ),
        _data( d )
      { }

    explicit Event_base( const Event_base& e ) :
        __Event_Base( e ),
        _data( e._data )
      { }

    const_reference value() const
      { return _data; }
    reference value()
      { return _data; }
    void value( const signed char& d )
      { _data = d; }
    size_type value_size() const
      { return sizeof(_data); }

    void net_pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        net_pack( ss );
        s.value( ss.str() );
      }

    void net_unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        net_unpack( std::stringstream( s.value() ) );
      }

    void pack( Event& s ) const
      {
        s.code( _code );
        s.dest( _dst );
        s.src( _src );
        s.sid( _sid );
        s.seq( _sqn );
        s.responce( _rsqn );
        std::stringstream ss;
        pack( ss );
        s.value( ss.str() );
      }

    void unpack( const Event& s )
      {
        _code = s.code();
        _dst  = s.dest();
        _src  = s.src();
        _sid  = s.sid();
        _sqn  = s.seq();
        _rsqn = s.responce();
        unpack( std::stringstream( s.value() ) );
      }

    void pack( std::ostream& __s ) const
      { __s.write( (const char *)&_data, sizeof(D) ); }
    void unpack( std::istream& __s )
      { __s.read( (char *)&_data, sizeof(D) ); }
    void net_pack( std::ostream& __s ) const
      {
        value_type tmp = to_net( _data );
        __s.write( (const char *)&tmp, sizeof(D) );
      }
    void net_unpack( std::istream& __s )
      {
        value_type tmp;
        __s.read( (char *)&tmp, sizeof(D) );
        _data = from_net( tmp );
      }

  protected:
    value_type _data;
};

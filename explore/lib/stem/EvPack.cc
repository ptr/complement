// -*- C++ -*- Time-stamp: <99/03/19 16:59:36 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <EvPack.h>
#include <iterator>

namespace EDS {

using std::string;
using std::istream;
using std::ostream;
using std::copy;
using std::ostream_iterator;
using std::char_traits;

void __pack_base::__net_unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, 4 );
  sz = from_net( sz );
  if ( sz > 0 ) {
    str.clear();
    str.reserve( sz );
    while ( sz-- > 0 ) {
      str += (char)s.get();
    }
  }
}

void __pack_base::__net_pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  sz = to_net( sz );
  s.write( (const char *)&sz, 4 );
  copy( str.begin(), str.end(), ostream_iterator<char,char,char_traits<char> >(s) );
}

void __pack_base::__unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, 4 );
  if ( sz > 0 ) {
    str.clear();
    str.reserve( sz );
    while ( sz-- > 0 ) {
      str += (char)s.get();
    }
  }
}

void __pack_base::__pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  s.write( (const char *)&sz, 4 );
  copy( str.begin(), str.end(), ostream_iterator<char,char,char_traits<char> >(s) );
}

} // namespace EDS

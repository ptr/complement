// -*- C++ -*- Time-stamp: <99/05/24 15:07:13 ptr>
#ident "$SunId$ %Q%"

#include <EvPack.h>
#include <iterator>

namespace EDS {

using std::string;
using std::istream;
using std::ostream;
using std::copy;

__DLLEXPORT
void __pack_base::__net_unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, 4 );
  sz = from_net( sz );
  if ( sz > 0 ) {
    str.erase();
    str.reserve( sz );
    while ( sz-- > 0 ) {
      str += (char)s.get();
    }
  }
}

__DLLEXPORT
void __pack_base::__net_pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  sz = to_net( sz );
  s.write( (const char *)&sz, 4 );
  copy( str.begin(), str.end(), std::ostream_iterator<char,char,std::char_traits<char> >(s) );
}

__DLLEXPORT
void __pack_base::__unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, 4 );
  if ( sz > 0 ) {
    str.erase();
    str.reserve( sz );
    while ( sz-- > 0 ) {
      str += (char)s.get();
    }
  }
}

__DLLEXPORT
void __pack_base::__pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  s.write( (const char *)&sz, 4 );
  copy( str.begin(), str.end(), std::ostream_iterator<char,char,std::char_traits<char> >(s) );
}

} // namespace EDS

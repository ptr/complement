
#include <iostream>

using namespace std;

unsigned char a64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

// void decode( istream )
// {
// }

void decodeblock( unsigned char in[4], unsigned char out[3] )
{
  out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
  out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
  out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

#if 1
void decode( istream& is, ostream& os )
{
  unsigned char in[4], out[3], v;
  int len;

  while( !is.eof() ) {
    len = 0;
    for( int i = 0; i < 4 && !is.eof(); i++ ) {
      v = 0;
      while( !is.eof() && v == 0 ) {
        is >> v;
        v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
        if ( v ) {
          v = (unsigned char) ((v == '$') ? 0 : v - 61);
        }
      }
      if( !is.eof() ) {
        len++;
        if ( v ) {
          in[ i ] = (unsigned char) (v - 1);
        }
      } else {
        in[i] = 0;
      }
    }
    if ( len ) {
      decodeblock( in, out );
      for ( int i = 0; i < len - 1; i++ ) {
        os << out[i];
      }
    }
  }
}
#else
void decode( istream& is, ostream& os )
{
  unsigned char in[4], out[3], v;
  while( !is.eof() ) {
    is >> in[0] >> in[1] >> in[2] >> in[3];
    decodeblock( in, out );
    os << out[0] << out[1] << out[2];
  }
}
#endif

int main()
{
  decode( cin, cout );

  return 0;
}

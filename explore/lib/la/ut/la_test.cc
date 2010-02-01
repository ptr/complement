#include "la_test.h"

#include <la/Integer.h>

#include <string>
#include <iostream>

using namespace std;

typedef la_int<1, long> Integer1; // equal to long
typedef la_int<2, long> Integer2;

int inrange( int l, int r, int x )
{
  return (x >= l) && (x <= r);
}

int EXAM_IMPL(la_test::one_char)
{
  typedef la_int<1, char> IntegerC;
  
  for (int i = CHAR_MIN;i <= CHAR_MAX;++i) {
    IntegerC x = (char)i;
    
    {
      ostringstream oss;
      oss << i;
      EXAM_CHECK( string(x) == oss.str() );
    }
    
    EXAM_CHECK( -x == -((char)i) ); 
    EXAM_CHECK( x == -(-x) );
    
    
    for (int j = CHAR_MIN;j <= CHAR_MAX;++j) {
      IntegerC y = (char)j;
      if ( inrange(CHAR_MIN, CHAR_MAX, i + j ) ) {
        EXAM_CHECK(x + y == char(i + j) );
      }
      if ( inrange(CHAR_MIN + 1, CHAR_MAX, i * j ) ) { // !!!
        EXAM_CHECK( x * y == char(i * j) );
      }
      
      if ( inrange(CHAR_MIN, CHAR_MAX, i - j ) ) {
        EXAM_CHECK(x - y == char(i - j) );
      }
  
      if (inrange(0,CHAR_MAX,i) && inrange(1,CHAR_MAX,j)) {
        EXAM_CHECK( x / y == (char)(i / j) );
        EXAM_CHECK( x % y == (char)(i % j) );
      }
      
    }
    
    for (int sh = 0;sh < 8 * sizeof(unsigned char);++sh) {
      EXAM_CHECK( (x << sh) == ((char)i) << sh );
    }
  }
  
  return EXAM_RESULT;
}

int EXAM_IMPL(la_test::one_short)
{
  typedef la_int<1, short> IntegerS;
  
  srand( 42 );
  for (int p = 0;p < 32;++p) {
    int i = short(rand());
    IntegerS x = (short)i;
    
    {
      ostringstream oss;
      oss << i;
      EXAM_CHECK( string(x) == oss.str() );
    }
    
    EXAM_CHECK( -x == -((short)i) ); 
    EXAM_CHECK( x == -(-x) );
    
    
    for (int j = SHRT_MIN;j <= SHRT_MAX;++j) {
      IntegerS y = (short)j;
      if ( inrange(SHRT_MIN, SHRT_MAX, i + j ) ) {
        EXAM_CHECK(x + y == short(i + j) );
      }
      if ( inrange(SHRT_MIN + 1, SHRT_MAX, i * j ) ) { // !!!
        EXAM_CHECK( x * y == short(i * j) );
      }
      
      if ( inrange(SHRT_MIN, SHRT_MAX, i - j ) ) {
        EXAM_CHECK(x - y == short(i - j) );
      }
  
      if (inrange(0,SHRT_MAX,i) && inrange(1,SHRT_MAX,j)) {
        EXAM_CHECK( x / y == (short)(i / j) );
        EXAM_CHECK( x % y == (short)(i % j) );
      }
      
    }
    
    for (int sh = 0;sh < 8 * sizeof(unsigned short);++sh) {
      EXAM_CHECK( (x << sh) == ((short)i) << sh );
    }
  }
  
  return EXAM_RESULT;  
}

string to_string( long long x ) {
  ostringstream oss;
  oss << x;
  return oss.str();
}

int EXAM_IMPL(la_test::two_chars)
{
  typedef la_int<2, char> IntegerS;
  
  srand( 42 );
  for (int p = 0;p < 16;++p) {
    int i = short(rand() % (1 << 15) - (1 << 14));

    IntegerS x;
    {
      ostringstream oss;
      oss << i;
      x.assign( oss.str() );
    }
    
    EXAM_CHECK( string(-x) == to_string(-i) ); 
    EXAM_CHECK( x == -(-x) );
    
    
    for (int j = -(1 << 14);j <= (1 << 14) - 1;++j) {
      IntegerS y;
      {
        ostringstream oss;
        oss << j;
        y.assign( oss.str() );
      }      
      if ( inrange(-(1 << 14), (1 << 14) - 1, i + j ) ) {
        EXAM_CHECK( string(x + y) == to_string(i + j) );
      }
    
      if ( inrange(-(1 << 14) + 1, (1 << 14) - 1, i * j ) ) { // !!!
        EXAM_CHECK( string(x * y) == to_string(i * j) );
      }
        
      if ( inrange(-(1 << 14), (1 << 14) - 1, i - j ) ) {
        EXAM_CHECK( string(x - y) == to_string(i - j) );
      }
         
      if (inrange(0,(1 << 14) - 1,i) && inrange(1,(1 << 14) - 1,j)) {
        EXAM_CHECK( string(x / y) == to_string(i / j) );
        EXAM_CHECK( string(x % y) == to_string(i % j) );

      }
    }
  }
  
  return EXAM_RESULT;  
}

int EXAM_IMPL(la_test::boundary_32bit)
{
  Integer4 x,y;
  Integer4 z;
  
  x.assign("1073741824"); // 2^30
  y = x;
  EXAM_CHECK( x / y == 1 );
  
  x.assign( "2147483648" ); // 2^31
  y = x;  
  EXAM_CHECK( x / y == 1 );

   
  return EXAM_RESULT;
}

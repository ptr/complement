// -*- C++ -*- Time-stamp: <02/09/25 12:11:17 ptr>

#include <iostream>
#include <locale>

using namespace std;

int main( int argc, char * const *argv )
{
  // cerr << "Hello, world!" << endl;
  typedef codecvt<char,char,mbstate_t> my_facet;

  locale loc_ref;
  {
    locale gloc( loc_ref, new my_facet );

    //The following code is just here to try to confuse the reference counting underlying mecanism:
    //locale::global( locale::classic() );
    //locale::global( gloc );
  }

  return 0;
}

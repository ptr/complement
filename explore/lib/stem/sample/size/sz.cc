// -*- C++ -*- Time-stamp: <07/10/15 22:44:45 ptr>

#include <stem/EventHandler.h>
#include <stem/EvManager.h>

#include <mt/xmt.h>

#include <iostream>
#include <string>

using namespace std;
using namespace xmt;
using namespace stem;

class StSample :
    public stem::EventHandler
{
  public:
    StSample();
    StSample( stem::addr_type id, const char *info = 0 );
    StSample( const char *info );
    ~StSample();

  private:
    void func( const stem::Event& );


    DECLARE_RESPONSE_TABLE( StSample, stem::EventHandler );
};

StSample::StSample() :
    stem::EventHandler()
{
}

StSample::StSample( stem::addr_type id, const char *info ) :
    stem::EventHandler( id, info )
{
}

StSample::StSample( const char *info ) :
    stem::EventHandler( info )
{
}

StSample::~StSample()
{
}

void StSample::func( const stem::Event& ev )
{
}

DEFINE_RESPONSE_TABLE( StSample )
  EV_EDS( ST_NULL, 0x200, func )
END_RESPONSE_TABLE

int main()
{
  for ( int i = 0; i < 50000; ++i ) {
    StSample *s = new StSample;
  }

  cout << "ready" << endl;

  string line;

  while ( cin.good() ) {
    getline( cin, line );
    if ( !cin.fail() ) {
      cerr << "local: " << line << endl;
    }
  }

  return 0;
}

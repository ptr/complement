// -*- C++ -*- Time-stamp: <07/02/28 10:41:37 ptr>

#ifndef __server_h
#define __server_h

#include <sockios/sockstream>

namespace intr {

class IncomeHttpRqProcessor //
{
  public:
    IncomeHttpRqProcessor( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

} // namespace intr

#endif // __server_h

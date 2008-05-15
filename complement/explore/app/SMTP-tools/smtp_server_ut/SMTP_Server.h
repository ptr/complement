// -*- C++ -*- Time-stamp: <08/04/25 14:35:17 yeti>

#ifndef __SMPT_SERVER_H
#define __SMPT_SERVER_H

#include <string>
#include <iostream>

namespace smtp {

enum state
{
  disconnect, 
  connect, 
  hello, 
  sender, 
  recipient, 
  letter
};

enum command
{
  helo, 
  ehlo, 
  mail, 
  rcpt, 
  data, 
  rset, 
  vrfy, 
  expn, 
  help, 
  noop, 
  quit, 
  none
};

class session 
{
  private:
    state st;
    command com;
    std::string message;

    std::basic_istream<char>& in;
    std::basic_ostream<char>& out;

    command setCom ( const std::string& str );
    void changeSt ( const std::string& str, const std::string& param, std::string& stout );
  public:
    session ( std::basic_istream<char>& is, std::basic_ostream<char>& os );
    ~session ()
      { };

    int checkData ();
    
    state getState ();
};

} // namespace smtp

#endif // __SMPT_SERVER_H

// -*- C++ -*- Time-stamp: <08/04/25 13:11:53 yeti>

#ifndef __SMPT_SERVER_H
#define __SMPT_SERVER_H

#include <string>

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

int ServerWork();

command setCom( const std::string& str );

void change( state& st, command& com, std::string& param, std::string& stout );

} // namespace smtp

#endif // __SMPT_SERVER_H

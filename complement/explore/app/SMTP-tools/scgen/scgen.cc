// -*- C++ -*- Time-stamp: <03/10/06 18:10:41 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: scgen.cc,v 1.3 2003/10/06 15:33:52 ptr Exp $"
#  else
#ident "@(#)$Id: scgen.cc,v 1.3 2003/10/06 15:33:52 ptr Exp $"
#  endif
#endif

#include <iostream>
#include "stategraph.h"

using namespace std;

enum vertex {
  NotConnected,
  Connected,
  Greeting,
  Mail,
  Recipient,
  Data,
  EndOfData,
  NVertices
};

enum edge {
  _connect_,
  _disconnect_,
  _QUIT_,
  _RSET_,
  _EXPN_,
  _HELP_,
  _VRFY_,
  _NOOP_,
  _MAIL_,
  _HELO_,
  _EHLO_,
  _RCPT_,
  _DATA_,
  _EOD_, // end of data
  _nothing_,
  _NCommands_
};


int main( int, char * const * )
{
  StateGraph g( NVertices, cout );

  g.edge( NotConnected, Connected, 5, _connect_ );
  g.edge( Connected, NotConnected, 10, _QUIT_ );
  g.edge( Connected, Connected, 1, _RSET_ );
  g.edge( Connected, Connected, 1, _EXPN_ );
  g.edge( Connected, Connected, 1, _HELP_ );
  g.edge( Connected, Connected, 1, _NOOP_ );
  g.edge( Connected, Connected, 1, _VRFY_ );
  g.edge( Connected, Greeting, 2, _HELO_ );
  g.edge( Connected, Greeting, 1, _EHLO_ );
  g.edge( Greeting, Greeting, 1, _EHLO_ );
  g.edge( Greeting, Greeting, 1, _RSET_ );
  g.edge( Greeting, Greeting, 1, _VRFY_ );
  g.edge( Greeting, Greeting, 1, _EXPN_ );
  g.edge( Greeting, Greeting, 1, _HELP_ );
  g.edge( Greeting, Greeting, 1, _NOOP_ );
  g.edge( Greeting, NotConnected, 10, _QUIT_ );
  g.edge( Greeting, Mail, 1, _MAIL_ );
  g.edge( Mail, Mail, 1, _NOOP_ );
  g.edge( Mail, Mail, 1, _HELP_ );
  g.edge( Mail, Greeting, 3, _EHLO_ );
  g.edge( Mail, Greeting, 3, _RSET_ );
  g.edge( Mail, NotConnected, 10, _QUIT_ );
  g.edge( Mail, Recipient, 1, _RCPT_ );
  g.edge( Recipient, Greeting, 3, _RSET_ );
  g.edge( Recipient, Greeting, 3, _EHLO_ );
  g.edge( Recipient, NotConnected, 10, _QUIT_ );
  g.edge( Recipient, Recipient, 1, _NOOP_ );
  g.edge( Recipient, Recipient, 1, _HELP_ );
  g.edge( Recipient, Data, 1, _DATA_ );
  g.edge( Data, EndOfData, 1, _EOD_ );
  g.edge( EndOfData, Greeting, 1, _nothing_ );
  
  string host( "$1 $2" );
  string hello( "$3" );
  string msg_to( "<$5>" );
  string msg_from( "<$4>" );

  g[_connect_] << "# --------------\n"
               << "connect " << host << "\n"
               << "expected 220\n";
  g[_disconnect_] << "disconnect\n";
  g[_QUIT_] << "QUIT\n"
            << "expected 221\n"
            << "disconnect\n"
            << "# --------------\n";
  g[_RSET_] << "RSET\n"
            << "expected 250\n";
  g[_EXPN_] << "EXPN some@host.com\n"
            << "expected\n";
  g[_HELP_] << "HELP\n"
            << "expected 250\n";
  g[_VRFY_] << "VRFY some@host.com\n"
            << "expected\n";
  g[_NOOP_] << "NOOP\n"
            << "expected 250\n";
  g[_MAIL_] << "MAIL FROM:" << msg_from << "\n"
            << "expected 250\n";
  g[_HELO_] << "HELO " << hello << "\n"
            << "expected 250\n";
  g[_EHLO_] << "EHLO " << hello << "\n"
            << "expected 250\n";
  g[_RCPT_] << "RCPT TO:" << msg_to << "\n"
            << "expected 250\n";
  g[_DATA_] << "DATA\n"
            << "expected 354\n"
            << ">>>A\n"
            << "Subject: KL smtp relay test\n\nSample mess\n";
  g[_EOD_]  // << "\r\n.\r\n"
            << "<<<A\n"
            << "expected 250\n";

  g.girdle( NotConnected );

  return 0;
}

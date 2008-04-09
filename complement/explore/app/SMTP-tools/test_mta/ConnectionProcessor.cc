// -*- C++ -*- Time-stamp: <04/07/09 18:18:30 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: ConnectionProcessor.cc,v 1.2 2005/05/10 15:18:06 ptr Exp $"
#  else
#ident "@(#)$Id: ConnectionProcessor.cc,v 1.2 2005/05/10 15:18:06 ptr Exp $"
#  endif
#endif

#include "ConnectionProcessor.h"
#include <string>
#include <iostream>
#include <sstream>

#ifndef _NO_TEST
# include <boost/test/unit_test.hpp>
# include "ut/message.h"
#endif

using namespace std;

bool trace_flag;

sockstream ConnectionProcessor::tecol;
string tecol_host;
int tecol_port;

ConnectionProcessor::ConnectionProcessor( std::sockstream& s )
{
  // BOOST_MESSAGE( "Server seen connection" );

#ifndef _NO_TEST
  BOOST_REQUIRE( s.good() );
#endif
  if ( trace_flag ) {
    cout << "!! connection established" << endl;
    cout << "-> 220 myhost ESMTP TestMTA" << endl;
  }
  // s << "220 myhost ESMTP TestMTA\n\r";
  s << "220 myhost SMTP TestMTA\n\r";
  s.flush();

  state = command;
}

void ConnectionProcessor::connect( std::sockstream& s )
{
#ifndef _NO_TEST
  // BOOST_MESSAGE( "Server start connection processing" );

  BOOST_REQUIRE( s.good() );

  string msg_tmp;

  getline( s, msg_tmp );

  BOOST_CHECK_EQUAL( msg_tmp, ::msg );
  BOOST_REQUIRE( s.good() );

  s << ::msg_rsp << endl; // server's response

  BOOST_REQUIRE( s.good() );
  // BOOST_MESSAGE( "Server stop connection processing" );
#else

  string msg_tmp;
  getline( s, msg_tmp );
  if ( state == command ) {
    stringstream ss( msg_tmp );

    string arg;

    ss >> arg;

    if ( arg == "EHLO" ) {
      if ( trace_flag ) {
        cout << "<- EHLO\n";
        cout << "-> 250 peak.avp.ru Hello" << endl;
      }
      // s << 250 << " peak.avp.ru Hello newbie.avp.ru [172.16.10.25], please\r\n";
      s << "250-peak.avp.ru Hello newbie.avp.ru [172.16.10.25], pleased to meet you\r\n"
        << "250-ENHANCEDSTATUSCODES\r\n"
        << "250-PIPELINING\r\n"
        << "250-8BITMIME\r\n"
        << "250-SIZE\r\n"
        << "250-DSN\r\n"
        << "250-ETRN\r\n"
        << "250-AUTH DIGEST-MD5 CRAM-MD5\r\n"
        << "250-DELIVERBY\r\n"
        << "250 HELP\r" << endl;

      // s.flush();
      return;
    } else if ( arg == "HELO" ) {
      if ( trace_flag ) {
        cout << "<- HELO\n";
        cout << "-> 250 peak.avp.ru Hello" << endl;
      }
      s << 250 << " peak.avp.ru Hello newbie.avp.ru [172.16.10.25], please\r\n";
      s.flush();
    } else if ( arg == "MAIL" ) {
      if ( trace_flag ) {
        cout << "<- MAIL\n";
        cout << "-> 250 Ok" << endl;
      }
      s << 250 << " Ok\r\n";
      s.flush();
      return;   
    } else if ( arg == "RCPT" ) {
      if ( trace_flag ) {
        cout << "<- RCPT\n";
        cout << "-> 250 Ok" << endl;
      }
      s << 250 << " Ok\r\n";
      s.flush();
      return;   
    } else if ( arg == "QUIT" ) {
      if ( trace_flag ) {
        cout << "<- QUIT\n";
        cout << "-> 221 Close connection" << endl;
      }
      s << 221 << " Close connection\r\n";
      s.flush();
      s.close();
      return;   
    } else if ( arg == "DATA" ) {
      if ( trace_flag ) {
        cout << "<- DATA\n";
        cout << "-> 354 Ok, send message body" << endl;
      }
      s << 354 << " Ok, send message body\r\n";
      s.flush();
      state = data;
      return;   
    } else if ( arg == "RSET" ) {
      if ( trace_flag ) {
        cout << "<- RSET\n";
        cout << "-> 250 Ok" << endl;
      }
      s << 250 << " Ok\r\n";
      s.flush();
    } else {
      if ( trace_flag ) {
        cout << "!! Unknown command:\n"
             << "<- " << msg_tmp << endl;
      }
    }
  } else if ( state == data ) {
    if ( msg_tmp == ".\r" || msg_tmp == ".\n" || msg_tmp == ".\r\n" ) {
      if ( trace_flag ) {
        cout << "<- .\n"
             << "-> 250 Ok" << endl;
      }
      s << 250 << " Ok\r\n";
      s.flush();
      state = command;
      return;   
    } else {
      string::size_type p = msg_tmp.find( "X-smtpgw-test:" );
      if ( p != string::npos ) {
        string tmp = msg_tmp.substr( p + string( "X-smtpgw-test:" ).length() );
        if ( trace_flag ) {
          cout << "!! Header: " << tmp << endl;
        }
        if ( !tecol.good() ) {
          if ( tecol.is_open() ) {
            tecol.close();
          }
          tecol.clear();
        }
        if ( !tecol.is_open() ) {
          tecol.open( tecol_host.c_str(), tecol_port );
        }
        tecol << tmp << endl;
        if ( trace_flag && tecol.fail() ) {
          cout << "!! problems with tecol server" << endl;
        }
      }
    }
  }


  
#endif

  return;
}

void ConnectionProcessor::close()
{
  // BOOST_MESSAGE( "Server: client close connection" );
  if ( trace_flag ) {
    cout << "!! connection closed" << endl;
  }
}

// -*- C++ -*- Time-stamp: <08/04/25 14:45:59 yeti>

#include <iostream>
#include <string>

#include "SMTP_Server.h"

namespace smtp {

using namespace std;

command setCom( const string& str )
{
  string Str;

  for ( string::const_iterator i = str.begin(); i != str.end(); ++i ) {
    Str += tolower( *i );
  }

  if ( Str == "helo" ) {
    return helo;
  }

  if ( Str == "ehlo" ) {
    return ehlo;
  }

  if ( Str == "mail" ) {
    return mail;
  }

  if ( Str == "rcpt" ) {
    return rcpt;
  }

  if ( Str == "data" ) {
    return data;
  }

  if (Str == "rset") {
    return rset;
  }

  if (Str == "vrfy") {
    return vrfy;
  }

  if (Str == "expn") {
    return expn;
  }

  if (Str == "help") {
    return help;
  }

  if (Str == "noop") {
    return noop;
  }

  if (Str == "quit") {
    return quit;
  }

  return none;
}

void change( state& st, const command& com, const string& param, string& stout )
{
  switch ( com ) {
    case helo:
      if (st == connect) {
        stout = "250 localhost Hello localhost, pleased to meet you";
        st = hello;
      } else {
        stout = "503 localhost Duplicate HELO/EHLO";
      }
      return;

    case ehlo:
      if (st == connect) {
        stout = "250-localhost Hello localhost, pleased to meet you\n";
        stout += "250-8BITMIME\n";
        stout += "250-SIZE 8000000\n";
        stout += "250 HELP";
        st = hello;
      } else {
        stout = "503 localhost Duplicate HELO/EHLO";
      }
      return;

    case mail:
      switch (st) {
        case connect:
          stout = "503 Polite people say HELO first";
          return;
        case hello:
          stout = "250 " + param + "... Sender ok";
          st = sender;
          return;
        case sender:
          stout = "503 Sender already specified";
          return;
        case recipient:
          stout = "503 Sender already specified";
          return;
      }
      break;

    case rcpt:
      switch (st) {
        case connect:
          stout = "503 Need MAIL before RCPT";
          return;
        case hello:
          stout = "503 Need MAIL before RCPT";
          return;
        case sender:
          stout = "250 " + param + "... Recipient ok";
          st = recipient;
          return;
        case recipient:
          stout = "250 " + param + "... Recipient ok";
          return;
      }
      break;

    case data:
      switch (st) {
        case connect:
          stout = "503 Need MAIL command";
          return;
        case hello:
          stout = "503 Need MAIL command";
          return;
        case sender:
          stout = "503 Need RCPT (recipient)";
          return;
        case recipient:
          stout = "354 Enter mail, end with '.' on a line by itself";
          st = letter;
          return;
      }
      break;

    case rset:
      stout = "250 Reset state";
      if ( st != connect ) {
        st = hello;
      }
      return;

    case vrfy:
      stout = "502 Command not implemented";
      return;

    case expn:
      stout = "502 Command not implemented";
      return;

    case help:
      stout = "214-This is SMTP_Server\n";
      stout += "214 End of HELP info";
      return;

    case noop:
      stout = "250 OK";
      return;

    case quit:
      stout = "221 localhost closing connection; ";
      stout += "Connection closed by foreign host.";
      st = disconnect;
      return;

    case none:
      stout = "500 Command unrecognized";
      return;
  }
}


int ServerWork()
{
  state st = connect;
  command com;
  string param, message, stout;

  while ( st != disconnect ) {
    if ( st != letter ) {
      string str;
      cin >> str;
      getline(cin, param);
      com = setCom(str);
      change(st, com, param, stout);
      cout << stout;
    } else {
      getline( cin, param );
      if ( param != "." ) {
        message += param + "\n";
      } else {
        st = hello;
        cout << message;
        message = "";
      }
    }
  }

  return 0;
}

} // namespace smtp

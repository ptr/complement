// -*- C++ -*- Time-stamp: <2010-11-11 14:06:05 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "syslog_test.h"

#include <exam/suite.h>
#include <string>
#include <fstream>
#include <sstream>
#include <sockios/syslog.h>
#include <mt/uid.h>
#include <mt/thread>

using namespace std;

int EXAM_IMPL(syslog_test::core_test)
{
 /* For this test required:

      - running syslogd (or other syslog server)

      - logging LOG_USER facility with priority LOG_ERR
        into local /var/log/user.log
        (i.e. present of line in /etc/syslog.conf like
        'user.* /var/log/user.log')
  */
  std::string s = xmt::uid();

  misc::use_syslog<LOG_ERR,LOG_USER>() << s << std::endl;

  ifstream l( "/var/log/user.log" );

  EXAM_REQUIRE( l.good() );

  string line;

  while ( l.good() ) {
    getline( l, line );
    if ( !l.fail() ) {
      if ( line.find( s ) != string::npos ) {
        /*
          Check that line is like

          Nov 11 13:59:29 void-ptr sockios_ut[26665]: 1f209230-7e8f-4cd8-803f-955109fee8e3
         */
        string::size_type f = line.find( '[' );

        EXAM_CHECK( f != string::npos );
        
        stringstream ss( line.substr( f + 1 ) );

        ss.unsetf( ios_base::skipws );
        pid_t pid;

        ss >> pid;

        EXAM_CHECK( pid == std::tr2::getpid() );

        char c;

        ss >> c;

        EXAM_CHECK( c == ']' );

        ss >> c;

        EXAM_CHECK( c == ':' );
       
        ss >> c;

        EXAM_CHECK( c == ' ' );

        break;
      }      
    }
  }

  EXAM_CHECK( !l.fail() );

  return EXAM_RESULT;
}

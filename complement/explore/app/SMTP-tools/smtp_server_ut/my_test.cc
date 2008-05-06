// -*- C++ -*- Time-stamp: <08/05/03 09:31:01 ptr>

#include "my_test.h"

#include "SMTP_Server.h"

#include <mt/thread>
#include <mt/mutex>
#include <mt/condition_variable>
#include <misc/type_traits.h>
#include <typeinfo>

#include <iostream>
#include <fstream>
#ifndef STLPORT
#  include <ext/stdio_filebuf.h>
#endif
#include <sstream>

#include <unistd.h>

using namespace std;
using namespace smtp;

//possible states
//        disconnect 
//        connect
//        hello
//        sender
//        recipient
//        letter

const string set_state[] = {
//  "quit\n",
  "",
  "helo mail.ru\n",
  "helo mail.ru\nmail from:sender@mail.ru\n",
  "helo mail.ru\nmail from:sender@mail.ru\nrcpt to:client@mail.ru\n",
//  "helo mail.ru\nmail from:sender@mail.ru\nrcpt to:client@mail.ru\ndata\n"
};
       
const string set_command[] = {
  "helo mail.ru", 
  "ehlo mail.ru", 
  "mail from:sender1@mail.ru", 
  "rcpt to:recepient@mail.ru", 
  "data", 
  "rset", 
  "vrfy sender@mail.ru", 
  "expn sender@mail.ru", 
  "help", 
  "noop", 
  "quit", 
  "none"
};

const int set_result[][sizeof(set_command)/sizeof(set_command[0])] = {
  {250, 250, 503, 503, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 250, 503, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 503, 250, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 503, 250, 354, 250, 502, 502, 214, 250, 221, 500},
};        

static int fd1[2], fd2[2];

typedef list<string> tests_container_type;
static tests_container_type tests;

void server_thread()
{

#ifdef STLPORT
  ifstream in( fd2[0] );
  ofstream out( fd1[1] );
#elif defined( __GNUC__ )
  __gnu_cxx::stdio_filebuf<char> _in_buf( fd2[0], std::ios_base::in );
  basic_istream<char> in( &_in_buf );

  __gnu_cxx::stdio_filebuf<char> _out_buf( fd1[1], std::ios_base::out );
  basic_ostream<char> out( &_out_buf );
#endif
  
  state st = connect;
  command com;
  string param, message, stout;

  while ( st != disconnect ) {
    if ( st != letter ) {
      string str;
      in >> str;
      if ( str.empty() ) 
        break;
      getline( in, param );
      com = setCom( str );
      change( st, com, param, stout );
      out << stout << endl;
    } else {
      getline( in, param );
      if ( param.empty() ) 
        break;
      if ( param != "." ) {
        message += param + "\n";
      } else {
        st = hello;
//        cerr << message;
        message = "";
      }
    }
  }
  
  close(fd2[0]);
  close(fd1[1]);
}

int EXAM_IMPL(my_test::thread_call)
{  
  string s;
  string expected;
  string result;
   
  for ( tests_container_type::const_iterator i = tests.begin(); i != tests.end(); ++i ) {
    pipe( fd1 );
    pipe( fd2 );

#ifdef STLPORT
    ifstream in( fd1[0] );
    ofstream out( fd2[1] );
#elif defined( __GNUC__ )
    __gnu_cxx::stdio_filebuf<char> _in_buf( fd1[0], std::ios_base::in );
    basic_istream<char> in( &_in_buf );

    __gnu_cxx::stdio_filebuf<char> _out_buf( fd2[1], std::ios_base::out );
    basic_ostream<char> out( &_out_buf );
#endif

    std::tr2::basic_thread<0,0> t( server_thread );

    istringstream in_tst( *i );
    getline( in_tst, expected );

    while ( in_tst.good() ){
      getline( in_tst, s ); 
      if ( !s.empty() ) {
        out << s << endl;
//        cerr << s << endl;
        do {
          getline (in, result); 
//          cerr << result << endl;
        } while ( result[3] == '-' );
      }
    }

    close(fd2[1]);
    close(fd1[0]);
    t.join();

    if ( expected.compare(0, 3, result, 0, 3) != 0 ) {
      cerr << expected << "!=" << result << " at " << *i << endl;
    }
        
    EXAM_CHECK( expected.compare(0, 3, result, 0, 3) == 0 );
  }      
  return EXAM_RESULT;
}


int EXAM_IMPL(my_test::test_gen)
{
  tests.push_back( string( "221\nhelo ya.ru\nhelp\nquit\n" ) ); // aka test_0

  for ( int i = 0; i < sizeof(set_state)/sizeof(set_state[0]); ++i ) {
    for ( int j = 0; j < sizeof(set_command)/sizeof(set_command[0]); ++j ) {
      stringstream os;

      os << set_result[i][j] << "\n" << set_state[i] << set_command[j];

      tests.push_back( os.str() );
    }
  }

  return EXAM_RESULT;
}

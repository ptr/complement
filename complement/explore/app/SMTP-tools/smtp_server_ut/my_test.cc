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

//possible commands
//        helo
//        ehlo
//        mail
//        rcpt
//        data
//        rset
//        vrfy
//        expn
//        help
//        noop
//        quit
//        none

        
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
//  {999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999},
  // {221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221},
  {250, 250, 503, 503, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 250, 503, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 503, 250, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 503, 250, 354, 250, 502, 502, 214, 250, 221, 500},
//  {999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999}
};        

const int buf_size = 1024;
const int com_length = 4;
const int test_num = 0;

static int fd1[2], fd2[2];

typedef list<string> tests_container_type;
static tests_container_type tests;

bool active;

void server_thread()
{
  char buffer[buf_size];
  state st = connect;
  command com;

#ifdef STLPORT
  ifstream in( fd2[0] );
  ofstream out( fd1[1] );
#elif defined( __GNUC__ )
  __gnu_cxx::stdio_filebuf<char> _in_buf( fd2[0], std::ios_base::in );
  basic_istream<char> in( &_in_buf );

  __gnu_cxx::stdio_filebuf<char> _out_buf( fd1[1], std::ios_base::out );
  basic_ostream<char> out( &_out_buf );
#endif


  string param, message, stout;
  while ( st != disconnect ) {
    if ( st != letter ) {
      string str;
      in >> str;
      getline(in, param);
      com = setCom(str);
      change(st, com, param, stout);
      write( fd1[1], stout.data(), stout.length() );
//      out << stout;
    } else {
      getline( in, param );
      if ( param != "." ) {
        message += param + "\n";
      } else {
        st = hello;
//        out << message;
        message = "";
      }
    }
  }


/*  while ( st != disconnect ) {
    if ( st != letter ) {
      bool full = false;
      stringstream st_stream;
      while (!full) {
        int n = read( fd2[0], buffer, sizeof(buffer) );
        if ( n < 0 ) {                                      
          cerr << "Reading error\n";
          break;
        } else if ( n == 0 ) {
          break;
        } else {
          buffer[n] = '\0';
          cerr << ">> " << buffer;
          st_stream << buffer;
          full = full || (buffer[n-1] == '\n');
        }
      };
      if (full) { 
        while (st_stream.good()) {
          string str;
          string param, stout;
          st_stream >> str;
          if (str != "") {
            getline (st_stream, param);
//          param.assign( str, com_length, str.size() );
//          str.erase( com_length, str.size() - com_length + 1 );
            com = setCom( str );
            change( st, com, param, stout );
            write( fd1[1], stout.data(), stout.length() );
          }
        }
      } else {                                    //error appeared while reading a command
        break;
      };
    } else {
      string message, s;
      int n = read( fd2[0], buffer, sizeof(buffer) );             
      if ( n < 0 ) {                                      
        cerr << "Reading error\n";
        break;
      } else if ( n == 0 ) {
        cerr << "Empty\n";
        break;
      } else {
        buffer[n] = '\0';
        s.assign (buffer, 0, n);         
        if ( s != "." ) {
          message += s + "\n";
        } else {
          st = hello;
          // cout << message;
          message = "";
        }
      }
    }
  }
*/
  
  close(fd2[0]);
  close(fd1[1]);
  active = false;
  // cerr << "Server's loop may be here" << endl;
}

int EXAM_IMPL(my_test::thread_call)
{  
  active = false;

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
    char r_buffer[buf_size], w_buffer[buf_size];

    std::tr2::basic_thread<0,0> t( server_thread );
    active = true;

    istringstream in_tst( *i );

    getline( in_tst, expected );

    while ( in_tst.good() ){
      getline( in_tst, s ); 
      if ( !s.empty() ) {
        out << s << endl; // write( fd2[1], s.data(), s.length() );
        // write( fd2[1], "\n", 1 );
/*        int n = read( fd1[0], r_buffer, sizeof(r_buffer) );
        if ( n < 0 ) {                                      
          cerr << "Reading error\n";
          break;
        } else if ( n == 0 ) {
          cerr << "Empty string\n";
          break;
        } else {
          r_buffer[n] = '\0';
          cerr << "<< " << r_buffer;
        }
        out << s << "\n";
*/      cerr << s << "\n";
//        do {
          getline (in, result); 
          cerr << result;
//        } while (!result.empty());
//        if (result.empty()) cerr << "Empty";
      }
    }

//    string str ("helo mail.ru\nmail from:sender@mail.ru\nrcpt to:client@mail.ru\nquit\n");
//    write( fd2[1], str.data(), str.length() );

    close(fd2[1]);
    close(fd1[0]);

    t.join();

//    string result(r_buffer);
    if ( expected.compare(0, 3, result, 0, 3) != 0 ) {
      cerr << expected << "!=" << result << " at " << *i << endl;
    }
        
    EXAM_CHECK( expected.compare(0, 3, result, 0, 3) == 0 );
  }      
/*        strcpy (w_buffer, "help");
        write (fd2[1], w_buffer, sizeof(w_buffer));
        read (fd1[0], r_buffer, sizeof(r_buffer));
        cerr << r_buffer;
  
        strcpy (w_buffer, "quit");
        write (fd2[1], w_buffer, sizeof(w_buffer));
        read (fd1[0], r_buffer, sizeof(r_buffer));
        cerr << r_buffer;
    
        cerr << "Client's text may be here" << endl;
*/
  // EXAM_CHECK( val == 1 );
  // std::tr2::basic_thread<0,0> t2( thread_func_int, 2 );
  // t2.join();
  // EXAM_CHECK( val == 2 );
  // val = 0;

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

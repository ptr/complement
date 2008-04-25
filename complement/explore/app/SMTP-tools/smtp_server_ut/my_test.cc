// -*- C++ -*- Time-stamp: <08/04/25 15:23:35 yeti>

#include "my_test.h"


#include "SMTP_Server.h"

#include <mt/thread>
#include <mt/mutex>
#include <mt/condition_variable>
#include <misc/type_traits.h>
#include <typeinfo>

#include <iostream>
#include <fstream>
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
  "quit\n",
  "",
  "helo mail.ru\n",
  "helo mail.ru\nmail from:sender@mail.ru\n",
  "helo mail.ru\nmail from:sender@mail.ru\nrcpt to:client@mail.ru\n",
  "helo mail.ru\nmail from:sender@mail.ru\nrcpt to:client@mail.ru\ndata\n"
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
  {999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999},
  // {221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221},
  {250, 250, 503, 503, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 250, 503, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 503, 250, 503, 250, 502, 502, 214, 250, 221, 500},
  {503, 503, 503, 250, 354, 250, 502, 502, 214, 250, 221, 500},
  {999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999}
};        

const int buf_size = 1024;
const int com_length = 4;
const int test_num = 0;

static int fd1[2], fd2[2];

bool active;

void server_thread()
{
  char buffer[buf_size];
  state st = connect;
  command com;
  string param, message, stout;

  while ( st != disconnect ) {
    if ( st != letter ) {
      if ( read( fd2[0], buffer, sizeof(buffer) ) < 1 ) { // <-- error (1)
        cerr << "Reading error\n";
      } else {
        cerr << ">> " << buffer << endl;
        string str( buffer );                             // <-- error (1)
        param.assign( str, com_length, str.size() );
        str.erase( com_length, str.size() - com_length + 1 );
        com = setCom( str );
        change( st, com, param, stout );
        write( fd1[1], stout.data(), stout.length() );
      }
    } else {
      read( fd2[0], buffer, sizeof(buffer) );             // <-- error (2)
      param.assign (buffer, 0, sizeof(buffer) );          // <-- error (2)
      if ( param != "." ) {
        message += param + "\n";
      } else {
        st = hello;
        // cout << message;
        message = "";
      }
    }
  }

  active = false;
  // cerr << "Server's loop may be here" << endl;
}

int EXAM_IMPL(my_test::thread_call)
{
  pipe (fd1);
  pipe (fd2);
  
  active = false;

  string s;
  string expected;

  for ( int i = 0; i <= test_num; ++i ) {
    char r_buffer[buf_size], w_buffer[buf_size];

    std::tr2::basic_thread<0,0> t( server_thread );
    active = true;

    ostringstream st;
    st << "aux/test_" << i;

    ifstream in( st.str().c_str() );

    getline( in, expected );

    while ( in.good() ){
      getline( in, s ); 
      if ( !s.empty() ) {
        write( fd2[1], s.data(), s.length() );
        read( fd1[0], r_buffer, sizeof(r_buffer) ); // <-- error (3)
        cerr << "<< " << r_buffer;                  // <-- error (3)
      }
    }

    in.close();

    t.join();

    string result(r_buffer);
    if ( expected.compare(0, 3, result, 0, 3) != 0 ) {
      cerr << expected << "!=" << result << " at " << i << endl;
    }
        
    EXAM_CHECK( expected.compare(0, 3, result, 0, 3) == 0 );
        
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
  }
  // EXAM_CHECK( val == 1 );
  // std::tr2::basic_thread<0,0> t2( thread_func_int, 2 );
  // t2.join();
  // EXAM_CHECK( val == 2 );
  // val = 0;

  return EXAM_RESULT;
}


int EXAM_IMPL(my_test::test_gen)
{
  int num = 1;

  for ( int i = 0; i < sizeof(set_state)/sizeof(set_state[0]); ++i ) {
    for ( int j = 0; j < sizeof(set_command)/sizeof(set_command[0]); ++j ) {
      ostringstream st;
      st << "aux/test_" << num++;

      ofstream os( st.str().c_str() );

      os << set_result[i][j] << "\n"
         << set_state[i] << set_command[j];
    }
  }

  return EXAM_RESULT;
}

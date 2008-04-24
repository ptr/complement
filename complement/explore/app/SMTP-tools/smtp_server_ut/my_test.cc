// -*- C++ -*- Time-stamp: <08/03/26 01:53:46 ptr>

#include "my_test.h"


#include "SMTP_Server.h"

#include <mt/thread>
#include <mt/mutex>
#include <mt/condition_variable>
#include <misc/type_traits.h>
#include <typeinfo>

#include <iostream>
#include <fstream>
#include <semaphore.h>

#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <unistd.h>

using namespace std;

//possible states
//        disconnect 
//        connect
//        hello
//        sender
//        recipient
//        letter

const string set_state [6] = {
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

        
const string set_command [12] = {
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

const int set_result [6][12] = {
            {999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999, 999},
//            {221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221, 221},
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
	while (st != disconnect) {
		if (st != letter) {
		    if (read (fd2[0], buffer, sizeof(buffer)) < 1) fprintf(stderr,"Reading error\n");
		    else {
		        cerr << ">> "<< buffer << endl;
    			string str (buffer);
    			param.assign (str, com_length, str.size());
    			str.erase (com_length, str.size() - com_length + 1);
    			com = setCom (str);
    			change (st, com, param, stout);
  
                strcpy (buffer, stout.c_str());
                write (fd1[1], buffer, sizeof(buffer));
            }
		}
		else {
		    read (fd2[0], buffer, sizeof(buffer));
			param.assign (buffer, 0, sizeof(buffer));
			if (param != ".") message = message + param + "\n";
			else {
				st = hello;
//				cout << message;
				message = "";
			}
		};
	};
	active = false;
//    cerr << "Server's loop may be here" << endl;
}

int EXAM_IMPL(my_test::thread_call)
{
    pipe (fd1);
    pipe (fd2);
    active = false;
    for (int i = 0; i <= test_num; i++) {
        char r_buffer[buf_size], w_buffer[buf_size];
        std::tr2::basic_thread<0,0> t( server_thread );
        active = true;

        ostringstream st;
        st << "test_" << i;
        ifstream in (st.str().c_str());
        
        string expected;
        getline(in, expected);
        
        while (!in.eof()){
            string s;
            getline(in, s); 
            if (s != "") {
                strcpy (w_buffer, s.c_str());
                write (fd2[1], w_buffer, sizeof(w_buffer));
                read (fd1[0], r_buffer, sizeof(r_buffer));
                cerr << "<< " << r_buffer;
            }
        }
        in.close();
        t.join();

        string result (r_buffer);
        if (expected.compare (0, 3, result, 0, 3) != 0) 
            cerr << expected << "!=" << result << " at " << i << endl;
        
        EXAM_CHECK ((expected.compare (0, 3, result, 0, 3) == 0));
        
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
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 12; j++) {
            ostringstream st;
            st << "test_" << num;
//            st << "test_" << i << "_" << j;
            ofstream os (st.str().c_str());
            os << set_result [i][j] << endl;
            os << set_state [i] << set_command [j];
            os.close();
            num++;
        }
    }
    return EXAM_RESULT;
}



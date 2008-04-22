// -*- C++ -*- Time-stamp: <08/03/26 01:53:46 ptr>

#include "my_test.h"
#include "SMTP_Server.h"

#include <mt/thread>
#include <mt/mutex>
#include <mt/condition_variable>
#include <misc/type_traits.h>
#include <typeinfo>

#include <iostream>
#include <semaphore.h>

#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <unistd.h>

using namespace std;

static int fd1[2], fd2[2];

const int buf_size = 1024;
const int com_length = 4;

void server_thread()
{
    char buffer[buf_size];
    state st = connect;
    command com;
    string param, message, stout;
  
	while (st != disconnect) {
		if (st != letter) {

		    if (read (fd2[0], buffer, sizeof(buffer)) < 1) fprintf(stderr,"Reading error\n");
		        else fprintf (stderr,"%s\n",buffer);
		    
			string str(buffer);
			param.assign (str, com_length, str.size());
			str.erase (com_length, str.size() - com_length + 1);
			com = setCom (str);
			change (st, com, param, stout);
  
            strcpy (buffer, stout.c_str());
            write (fd1[1], buffer, sizeof(buffer));
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
    cerr << "Server's loop may be here" << endl;
}

int EXAM_IMPL(my_test::thread_call)
{
    char r_buffer[buf_size], w_buffer[buf_size];
    pipe (fd1);
    pipe (fd2);
    std::tr2::basic_thread<0,0> t( server_thread );

    strcpy (w_buffer, "ehlo");
    write (fd2[1], w_buffer, sizeof(w_buffer));
    read (fd1[0], r_buffer, sizeof(r_buffer));
    cerr << r_buffer;

    strcpy (w_buffer, "help");
    write (fd2[1], w_buffer, sizeof(w_buffer));
    read (fd1[0], r_buffer, sizeof(r_buffer));
    cerr << r_buffer;
  
    strcpy (w_buffer, "quit");
    write (fd2[1], w_buffer, sizeof(w_buffer));
    read (fd1[0], r_buffer, sizeof(r_buffer));
    cerr << r_buffer;
    
    cerr << "Client's text may be here" << endl;
    t.join();

    // EXAM_CHECK( val == 1 );
    // std::tr2::basic_thread<0,0> t2( thread_func_int, 2 );
    // t2.join();
    // EXAM_CHECK( val == 2 );
    // val = 0;

    return EXAM_RESULT;
}


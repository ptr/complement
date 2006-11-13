#include <sys/ipc.h>
#include <sys/msg.h>

#include <iostream>
#include <iomanip>

using namespace std;

int main( int argc, char **argv )
{
  // key_t key = 12345;
  key_t key = ftok( "/tmp/msg.queue", 276 );

  int msqid = msgget( key /* IPC_PRIVATE */, 0666 | IPC_CREAT );

  if ( msqid < 0 ) {
    cerr << "Can't create message queue" << endl;
    return 1;
  }


  struct msqid_ds buf;
  if ( msgctl( msqid, IPC_STAT, &buf ) < 0 ) {
    cerr << "Can't stat message queue" << endl;
    return 2;
  }

  cout << buf.msg_perm.uid << "\n" << buf.msg_perm.gid << "\n"
       << oct << buf.msg_perm.mode << dec << "\n"
       << buf.msg_qbytes << endl;

  // if ( msgctl( msqid, IPC_RMID, &buf ) ) {
  //   cerr << "Can't remove message queue" << endl;
  //   return 4;
  // }

  return 0;


  buf.msg_qbytes = /* 16384; */ 256 * 1024 * 1024;

  if ( msgctl( msqid, IPC_SET, &buf ) < 0 ) {
    cerr << "Can't set message queue params" << endl;
    return 3;
  }
  
  if ( msgctl( msqid, IPC_STAT, &buf ) < 0 ) {
    cerr << "Can't stat message queue" << endl;
    return 2;
  }

  return 0;
}

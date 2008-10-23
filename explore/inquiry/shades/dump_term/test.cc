#include <mt/xmt.h>
#include <iostream>

using namespace std;
using namespace xmt;

void h1( int sig )
{
  cerr << "Signal: " << sig << endl;
}

void h2( int sig, siginfo_t *si, void * )
{
  cerr << "-----------------------------------------------\n"
       << "my pid: " << xmt::getpid() << ", ppid: " << xmt::getppid() << "\n"
       << "signal: " << sig << ", number " << si->si_signo
       << " errno " << si->si_errno
       << " code " << si->si_code << endl;
  switch ( si->si_signo ) {
    case SIGKILL:
    case SIGTERM:
    case SIGINT:
    case SIGQUIT:
    case SIGPIPE:
    case SIGABRT:
    case SIGALRM:
    case SIGHUP:
      cerr << "pid: " << si->si_pid
           << "\nuid: " << si->si_uid
           << endl;
      break;
    case SIGILL:
    case SIGFPE:
    case SIGSEGV:
    case SIGBUS:
      cerr << "Address: " << si->si_addr << endl;
      break;
  }
}

class Superviser
{
  public:
    ~Superviser();
};

Superviser::~Superviser()
{
  cerr << "Good bye" << endl;
  kill( xmt::getpid(), SIGABRT );
}

// Superviser s;

int main()
{
  Condition cnd;

  cnd.set( false );
  signal_handler( SIGTERM, &h2 );
  signal_handler( SIGKILL, &h2 );
  signal_handler( SIGQUIT, &h2 );
  signal_handler( SIGINT, &h2 );
  signal_handler( SIGHUP, &h2 );
  signal_handler( SIGALRM, &h2 );
  signal_handler( SIGABRT, &h2 );
  signal_handler( SIGPIPE, &h2 );
  // signal_handler( SIGKILL, &h1 );
  // signal_handler( SIGSTOP, &h1 );
  // unblock_signal( SIGTERM );

  cerr << "Hello, world!" << endl;

  cnd.wait();

  return 0;
}

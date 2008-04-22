using namespace std;

enum state {
        disconnect, 
        connect, 
        hello, 
        sender, 
        recipient, 
        letter
     };
enum command {
        helo, 
        ehlo, 
        mail, 
        rcpt, 
        data, 
        rset, 
        vrfy, 
        expn, 
        help, 
        noop, 
        quit, 
        none
     };


int ServerWork();

command setCom(const string& str);

void change(state& st, command& com, string& param, string& stout);


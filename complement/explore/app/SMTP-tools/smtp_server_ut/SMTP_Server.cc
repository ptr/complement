#include <iostream>
#include <string>
#include "SMTP_Server.h"

using namespace std;

command setCom(const string& str) {
    string Str(str);
    for(int i = 0; Str[i] != '\0'; i++) 
		Str[i] = tolower(Str[i]);
	if (Str == "helo") return helo;
	else if (Str == "ehlo") return ehlo;
	else if (Str == "mail") return mail;
	else if (Str == "rcpt") return rcpt;
	else if (Str == "data") return data;
	else if (Str == "rset") return rset;
	else if (Str == "vrfy") return vrfy;
	else if (Str == "expn") return expn;
	else if (Str == "help") return help;
	else if (Str == "noop") return noop;
	else if (Str == "quit") return quit;
	else return none;
}

void change(state& st, command& com, string& param, string& stout) {
	switch (com) {
		case helo:
			if (st == connect) {
				stout = "250 localhost Hello localhost, pleased to meet you\n";
				st = hello;
				return;
			}
			else {
				stout = "503 localhost Duplicate HELO/EHLO\n";
				return;
			}
		case ehlo:
			if (st == connect) {
				stout = "250-localhost Hello localhost, pleased to meet you\n";
				stout += "250-8BITMIME\n";
				stout += "250-SIZE 8000000\n";
				stout += "250 HELP\n";
				st = hello;
				return;
			}
			else {
				stout = "503 localhost Duplicate HELO/EHLO\n";
				return;
			}
		case mail:
			switch (st) {
				case connect:
					stout = "503 Polite people say HELO first\n";
					return;
				case hello:
					stout = "250 " + param + "... Sender ok\n";
					st = sender;
					return;
				case sender:
					stout = "503 Sender already specified\n";
					return;
				case recipient:
					stout = "503 Sender already specified\n";
					return;
			}
		case rcpt:
			switch (st) {
				case connect:
					stout = "503 Need MAIL before RCPT\n";
					return;
				case hello:
					stout = "503 Need MAIL before RCPT\n";
					return;
				case sender:
					stout = "250 " + param + "... Recipient ok\n";
					st = recipient;
					return;
				case recipient:
					stout = "250 " + param + "... Recipient ok\n";
					return;
			}
		case data:
			switch (st) {
				case connect:
					stout = "503 Need MAIL command\n";
					return;
				case hello:
					stout = "503 Need MAIL command\n";
					return;
				case sender:
					stout = "503 Need RCPT (recipient)\n";
					return;
				case recipient:
					stout = "354 Enter mail, end with '.' on a line by itself\n";
					st = letter;
					return;
			}
		case rset:
			stout = "250 Reset state\n";
			if (st!=connect) st = hello;
			return;
		case vrfy:
			stout = "502 Command not implemented\n";
			return;
		case expn:
			stout = "502 Command not implemented\n";
			return;
		case help:
			stout = "214-This is SMTP_Server\n";
			stout += "214 End of HELP info\n";
			return;
		case noop:
			stout = "250 OK\n";
			return;
		case quit:
			stout = "221 localhost closing connection\n";
			stout += "Connection closed by foreign host.\n";
			st = disconnect;
			return;
		case none:
			stout = "500 Command unrecognized\n";
			return;
	}
}


int ServerWork() {
    state st = connect;
    command com;
    string param, message, stout;
	while (st != disconnect) {
		if (st != letter) {
			string str;
			cin >> str;
			getline(cin, param);
			com = setCom(str);
			change(st, com, param, stout);
			cout << stout;
		}
		else {
			getline(cin, param);
			if (param != ".") message = message + param + "\n";
			else {
				st = hello;
				cout << message;
				message = "";
			}
		};
	};
	return 0;
}
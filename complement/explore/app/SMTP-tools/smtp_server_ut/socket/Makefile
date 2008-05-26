all: server client

server: server.o socket.o
	gcc -g -o server server.o socket.o -lstdc++ -lpthread 
server.o: ECHO_server.cc socket.h
	gcc -g -c ECHO_server.cc -o server.o
client: client.o socket.o
	gcc -g -o client client.o socket.o -lstdc++
client.o: client.cc socket.h
	gcc -g -c client.cc -o client.o
socket.o: socket.cc socket.h
	gcc -g -c socket.cc -o socket.o

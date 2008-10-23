#!/bin/sh

c++ -pthread -g -c -I${HOME}/STLport.lab/STLport/stlport -o main.o main.cc
# c++ -pthread -g -c -I${HOME}/STLport.lab/STLport/stlport -o other.o other.cc
gcc -pthread -g -L${HOME}/STLport.lab/STLport/lib -Wl,--rpath=${HOME}/STLport.lab/STLport/lib -o cxxtest main.o -lstlport
# gcc -pthread -g -L${HOME}/STLport.lab/STLport/lib -Wl,--rpath=${HOME}/STLport.lab/STLport/lib -o cxxtest main.o other.o -lstlport
# c++ -pthread -g -I${HOME}/STLport.lab/STLport/stlport -L${HOME}/STLport.lab/STLport/lib -Wl,--rpath=${HOME}/STLport.lab/STLport/lib -o cxxtest main.o other.o -lstlport

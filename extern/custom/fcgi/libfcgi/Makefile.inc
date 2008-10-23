# -*- makefile -*- Time-stamp: <08/04/29 12:12:25 yeti>

FCGI_SRC = /home/yeti/fcgi-2.4.0/libfcgi
LIBNAME = fcgi
MAJOR = 2
MINOR = 4
PATCH = 0
SRC_C = ${FCGI_SRC}/fcgiapp.c \
        ${FCGI_SRC}/fcgi_stdio.c \
        ${FCGI_SRC}/os_unix.c

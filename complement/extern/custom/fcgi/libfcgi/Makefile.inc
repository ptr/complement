# -*- makefile -*- Time-stamp: <08/04/28 19:49:29 yeti>

FCGI_SRC = /export/hostel/labs/FastCGI/fcgi-2.4.0/libfcgi
LIBNAME = fcgi
MAJOR = 2
MINOR = 4
PATCH = 0
SRC_C = ${FCGI_SRC}/fcgiapp.c \
        ${FCGI_SRC}/fcgi_stdio.c \
        ${FCGI_SRC}/os_unix.c

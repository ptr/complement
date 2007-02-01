# -*- Makefile -*- Time-stamp: <07/02/01 19:53:51 ptr>

LIBNAME = sockios
MAJOR = 1
MINOR = 10
PATCH = 4
SRC_CC = _sockstream.cc _sockmgr.cc
SRC_C = freebsd/getaddrinfo.c \
        freebsd/ns_parse.c \
        freebsd/res_comp.c \
        freebsd/res_data.c \
        freebsd/res_init.c \
        freebsd/res_mkquery.c \
        freebsd/res_query.c \
        freebsd/res_send.c \
        freebsd/res_update.c \
        freebsd/nsdispatch.c

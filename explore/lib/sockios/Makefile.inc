# -*- Makefile -*- Time-stamp: <06/11/29 18:47:25 ptr>

LIBNAME = sockios
MAJOR = 1
MINOR = 10
PATCH = 1
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

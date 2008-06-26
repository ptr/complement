# -*- Makefile -*- Time-stamp: <08/03/05 23:38:15 ptr>

LIBNAME = sockios
MAJOR = 1
MINOR = 14
PATCH = 2
SRC_CC = _sockstream.cc _sockmgr.cc netinfo.cc _sp.cc
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

# -*- Makefile -*- Time-stamp: <08/12/18 13:32:34 ptr>

LIBNAME = sockios
MAJOR = 2
MINOR = 2
PATCH = 2
SRC_CC = _sockstream.cc _sockmgr.cc netinfo.cc
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

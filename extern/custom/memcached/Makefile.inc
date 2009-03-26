# -*- makefile -*- Time-stamp: <09/03/18 10:37:05 ptr>

MEMCACHED_SRC = ../../memcached
PRGNAME = memcached
SRC_C = $(MEMCACHED_SRC)/memcached.c \
        $(MEMCACHED_SRC)/hash.c \
        $(MEMCACHED_SRC)/slabs.c \
        $(MEMCACHED_SRC)/items.c \
        $(MEMCACHED_SRC)/assoc.c \
        $(MEMCACHED_SRC)/thread.c \
        $(MEMCACHED_SRC)/daemon.c \
        $(MEMCACHED_SRC)/stats.c

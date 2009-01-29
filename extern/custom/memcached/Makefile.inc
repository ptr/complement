# -*- makefile -*- Time-stamp: <09/01/29 10:04:27 ptr>

MEMCACHED_SRC = ${HOME}/labs/memcached
PRGNAME = memcached
SRC_C = $(MEMCACHED_SRC)/memcached.c \
        $(MEMCACHED_SRC)/slabs.c \
        $(MEMCACHED_SRC)/items.c \
        $(MEMCACHED_SRC)/assoc.c \
        $(MEMCACHED_SRC)/thread.c \
        $(MEMCACHED_SRC)/stats.c

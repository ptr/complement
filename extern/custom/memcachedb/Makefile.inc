# -*- makefile -*- Time-stamp: <09/03/18 10:37:05 ptr>

MEMCACHEDB_SRC = ../../memcachedb
PRGNAME = memcachedb
SRC_C = $(MEMCACHEDB_SRC)/memcachedb.c \
        $(MEMCACHEDB_SRC)/item.c \
        $(MEMCACHEDB_SRC)/thread.c \
        $(MEMCACHEDB_SRC)/bdb.c \
        $(MEMCACHEDB_SRC)/stats.c

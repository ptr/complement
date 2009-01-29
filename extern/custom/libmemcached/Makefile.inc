# -*- makefile -*- Time-stamp: <09/01/29 11:10:24 ptr>

LIBMEMCACHED_SRC = ${HOME}/labs/libmemcached-0.25/libmemcached
LIBNAME = memcached
MAJOR = 0
MINOR = 25
PATCH = 0
SRC_C = $(LIBMEMCACHED_SRC)/crc.c \
        $(LIBMEMCACHED_SRC)/hsieh_hash.c \
        $(LIBMEMCACHED_SRC)/memcached.c \
        $(LIBMEMCACHED_SRC)/memcached_auto.c \
        $(LIBMEMCACHED_SRC)/memcached_behavior.c \
        $(LIBMEMCACHED_SRC)/memcached_callback.c \
        $(LIBMEMCACHED_SRC)/memcached_connect.c \
        $(LIBMEMCACHED_SRC)/memcached_delete.c \
        $(LIBMEMCACHED_SRC)/memcached_do.c \
        $(LIBMEMCACHED_SRC)/memcached_fetch.c \
        $(LIBMEMCACHED_SRC)/memcached_fetch_execute.c \
        $(LIBMEMCACHED_SRC)/memcached_flush.c \
        $(LIBMEMCACHED_SRC)/memcached_get.c \
        $(LIBMEMCACHED_SRC)/memcached_hash.c \
        $(LIBMEMCACHED_SRC)/memcached_hosts.c \
        $(LIBMEMCACHED_SRC)/memcached_io.c \
        $(LIBMEMCACHED_SRC)/memcached_purge.c \
        $(LIBMEMCACHED_SRC)/md5.c \
        $(LIBMEMCACHED_SRC)/memcached_key.c \
        $(LIBMEMCACHED_SRC)/memcached_quit.c \
        $(LIBMEMCACHED_SRC)/memcached_parse.c \
        $(LIBMEMCACHED_SRC)/memcached_response.c \
        $(LIBMEMCACHED_SRC)/memcached_result.c \
        $(LIBMEMCACHED_SRC)/memcached_server.c \
        $(LIBMEMCACHED_SRC)/memcached_storage.c \
        $(LIBMEMCACHED_SRC)/memcached_string.c \
        $(LIBMEMCACHED_SRC)/memcached_stats.c \
        $(LIBMEMCACHED_SRC)/memcached_strerror.c \
        $(LIBMEMCACHED_SRC)/memcached_verbosity.c \
        $(LIBMEMCACHED_SRC)/memcached_version.c \
        $(LIBMEMCACHED_SRC)/murmur_hash.c \
        $(LIBMEMCACHED_SRC)/jenkins_hash.c \
        $(LIBMEMCACHED_SRC)/byteorder.c

# -*- makefile -*- Time-stamp: <09/03/18 10:37:05 ptr>

LIBEVENT_SRC = ../../libevent/libevent
LIBNAME = event
MAJOR = 1
MINOR = 4
PATCH = 9
SRC_C = $(LIBEVENT_SRC)/event.c \
        $(LIBEVENT_SRC)/buffer.c \
        $(LIBEVENT_SRC)/evbuffer.c \
        $(LIBEVENT_SRC)/log.c \
        $(LIBEVENT_SRC)/evutil.c \
        $(LIBEVENT_SRC)/event_tagging.c \
        $(LIBEVENT_SRC)/http.c \
        $(LIBEVENT_SRC)/evdns.c \
        $(LIBEVENT_SRC)/evrpc.c \
        $(LIBEVENT_SRC)/strlcpy.c \
        $(LIBEVENT_SRC)/signal.c \
        $(LIBEVENT_SRC)/select.c \
        $(LIBEVENT_SRC)/poll.c \
        $(LIBEVENT_SRC)/epoll.c

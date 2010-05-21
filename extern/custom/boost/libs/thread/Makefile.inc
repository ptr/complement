# -*- makefile -*- Time-stamp: <09/07/18 00:01:42 ptr>

BOOST_SRC = $(BOOST_INCLUDE_DIR)/libs/thread/src
LIBNAME = boost_thread
MAJOR = 1
MINOR = 43
PATCH = 0

SRC_CPP = $(BOOST_SRC)/pthread/thread.cpp \
          $(BOOST_SRC)/pthread/once.cpp

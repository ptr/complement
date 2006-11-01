# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>
# $Id$

BOOST_REGEX_SRC = $(BOOST_INCLUDE_DIR)/libs/thread/src
LIBNAME = boost_thread
MAJOR = 1
MINOR = 31
PATCH = 0
SRC_CPP = $(BOOST_REGEX_SRC)/condition.cpp \
	  $(BOOST_REGEX_SRC)/mutex.cpp \
	  $(BOOST_REGEX_SRC)/recursive_mutex.cpp \
	  $(BOOST_REGEX_SRC)/thread.cpp \
	  $(BOOST_REGEX_SRC)/tss.cpp \
	  $(BOOST_REGEX_SRC)/xtime.cpp \
	  $(BOOST_REGEX_SRC)/once.cpp \
	  $(BOOST_REGEX_SRC)/exceptions.cpp \
	  $(BOOST_REGEX_SRC)/threadmon.cpp

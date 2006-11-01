# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>

BOOST_SRC = $(BOOST_INCLUDE_DIR)/libs/thread/src
LIBNAME = boost_thread
MAJOR = 1
MINOR = 33
PATCH = 1

SRC_CPP = $(BOOST_SRC)/condition.cpp \
	  $(BOOST_SRC)/mutex.cpp \
	  $(BOOST_SRC)/recursive_mutex.cpp \
	  $(BOOST_SRC)/thread.cpp \
          $(BOOST_SRC)/xtime.cpp \
          $(BOOST_SRC)/once.cpp \
          $(BOOST_SRC)/exceptions.cpp \
          $(BOOST_SRC)/barrier.cpp \
	  $(BOOST_SRC)/tss.cpp \
          $(BOOST_SRC)/tss_hooks.cpp \
          $(BOOST_SRC)/tss_dll \
          $(BOOST_SRC)/tss_pe

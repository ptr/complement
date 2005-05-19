# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>
# $Id$

BOOST_REGEX_SRC = $(BOOST_INCLUDE_DIR)/libs/regex/src
LIBNAME = boost_regex
MAJOR = 1
MINOR = 32
PATCH = 0
SRC_CPP = $(BOOST_REGEX_SRC)/c_regex_traits.cpp \
	  $(BOOST_REGEX_SRC)/c_regex_traits_common.cpp \
	  $(BOOST_REGEX_SRC)/cpp_regex_traits.cpp \
	  $(BOOST_REGEX_SRC)/cregex.cpp \
	  $(BOOST_REGEX_SRC)/fileiter.cpp \
	  $(BOOST_REGEX_SRC)/instances.cpp \
	  $(BOOST_REGEX_SRC)/posix_api.cpp \
	  $(BOOST_REGEX_SRC)/regex.cpp \
	  $(BOOST_REGEX_SRC)/regex_debug.cpp \
	  $(BOOST_REGEX_SRC)/regex_synch.cpp \
	  $(BOOST_REGEX_SRC)/w32_regex_traits.cpp \
	  $(BOOST_REGEX_SRC)/wide_posix_api.cpp \
	  $(BOOST_REGEX_SRC)/winstances.cpp

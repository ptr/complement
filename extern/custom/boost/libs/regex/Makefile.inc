# -*- makefile -*- Time-stamp: <09/07/17 23:37:40 ptr>

BOOST_REGEX_SRC = $(BOOST_INCLUDE_DIR)/libs/regex/src
LIBNAME = boost_regex
MAJOR = 1
MINOR = 40
PATCH = 0
SRC_CPP = $(BOOST_REGEX_SRC)/c_regex_traits.cpp \
	  $(BOOST_REGEX_SRC)/regex_traits_defaults.cpp \
	  $(BOOST_REGEX_SRC)/cpp_regex_traits.cpp \
	  $(BOOST_REGEX_SRC)/cregex.cpp \
	  $(BOOST_REGEX_SRC)/fileiter.cpp \
	  $(BOOST_REGEX_SRC)/instances.cpp \
	  $(BOOST_REGEX_SRC)/posix_api.cpp \
	  $(BOOST_REGEX_SRC)/regex.cpp \
	  $(BOOST_REGEX_SRC)/regex_debug.cpp \
	  $(BOOST_REGEX_SRC)/regex_raw_buffer.cpp \
	  $(BOOST_REGEX_SRC)/w32_regex_traits.cpp \
	  $(BOOST_REGEX_SRC)/wide_posix_api.cpp \
	  $(BOOST_REGEX_SRC)/winstances.cpp \
          $(BOOST_REGEX_SRC)/icu.cpp \
          $(BOOST_REGEX_SRC)/wc_regex_traits.cpp \
          $(BOOST_REGEX_SRC)/static_mutex.cpp \
          $(BOOST_REGEX_SRC)/usinstances.cpp

# -*- makefile -*- Time-stamp: <08/04/29 11:13:44 yeti>

FASTCGIDAEMON_SRC = /export/hostel/labs/FastCGI/fastcgi-daemon-FCGID-1/library
LIBNAME = fastcgi-daemon
MAJOR = 0
MINOR = 0
PATCH = 0
SRC_CPP = ${FASTCGIDAEMON_SRC}/component.cpp \
          ${FASTCGIDAEMON_SRC}/config.cpp \
          ${FASTCGIDAEMON_SRC}/cookie.cpp \
          ${FASTCGIDAEMON_SRC}/except.cpp \
          ${FASTCGIDAEMON_SRC}/functors.cpp \
          ${FASTCGIDAEMON_SRC}/handler.cpp \
          ${FASTCGIDAEMON_SRC}/handlerset.cpp \
          ${FASTCGIDAEMON_SRC}/helpers.cpp \
          ${FASTCGIDAEMON_SRC}/loader.cpp \
          ${FASTCGIDAEMON_SRC}/logger.cpp \
          ${FASTCGIDAEMON_SRC}/parser.cpp \
          ${FASTCGIDAEMON_SRC}/range.cpp \
          ${FASTCGIDAEMON_SRC}/request.cpp \
          ${FASTCGIDAEMON_SRC}/requestimpl.cpp \
          ${FASTCGIDAEMON_SRC}/stream.cpp \
          ${FASTCGIDAEMON_SRC}/util.cpp \
          ${FASTCGIDAEMON_SRC}/xml.cpp \
          ${FASTCGIDAEMON_SRC}/componentset.cpp \
          ${FASTCGIDAEMON_SRC}/component_factory.cpp \
          ${FASTCGIDAEMON_SRC}/component_context.cpp

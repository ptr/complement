# -*- makefile -*- Time-stamp: <10/03/22 14:44:45 ptr>

SRCROOT := ../../../../explore

#LIBXMLXX_DIR = ${HOME}/labs/libxml++
#LIBXMLXX_SRC = ${LIBXMLXX_DIR}/libxml++
#GLIBMM_DIR = ${HOME}/labs/glibmm-2.23.3
# GLIBMM_SRC = ${GLIBMM_DIR}/glib/glibmm

#LIBSIGCXX_DIR = ${HOME}/labs/libsigc++2

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I.. -I${GLIBMM_DIR}/glib -I${LIBSIGCXX_DIR} -I${LIBXMLXX_DIR} -I../../libsigc++ -I../../glibmm/glib -I. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2

DEFS += -DLIBXMLPP_BUILD

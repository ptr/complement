# -*- makefile -*- Time-stamp: <10/03/18 09:38:45 ptr>

SRCROOT := ../../../../../explore
# POST_INSTALL = ln -sf ${SO_NAME} $(INSTALL_LIB_DIR)/libboost_date_time.so

GLIBMM_DIR = ${HOME}/labs/glibmm-2.23.3
GLIBMM_SRC = ${GLIBMM_DIR}/glib/glibmm

LIBSIGCXX_DIR = ${HOME}/labs/libsigc++2

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I.. -I${GLIBMM_DIR}/glib -I${LIBSIGCXX_DIR} -I../../../libsigc++ -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include


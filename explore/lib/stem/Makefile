# Time-stamp: <98/05/26 19:26:27 ptr>
# %Z% $Date$ $Revision$ $RCSfile$ %Q%
# Implementation and debugging EDS class library.

baselibname = EDS
libversion  = 3
libfiles_cc = EventsCore.cc EventHandler.cc EvManager.cc Application.cc
libname     = $(baselibname:%=lib%-debug.so)
DEFS = -D_PTHREADS
LDFLAGS =

include ../Makefile.lib

$(libname_v) := DEFS += -D__DEBUG=2 -D__TRACE -D__WARN
$(libname_v) := OPT = -g -pic +w
$(libname_v) := INCLUDES += -I/usr/local/include/tools
# $(libname_v) := LDLIBS += -L/usr/local/lib -lAuxd

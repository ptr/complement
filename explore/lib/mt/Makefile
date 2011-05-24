# -*- Makefile -*- Time-stamp: <2011-05-23 20:47:47 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include
HEADERS_BASE = $(SRCROOT)/include/mt $(SRCROOT)/include/config $(SRCROOT)/include/misc

ifdef WITHOUT_BFD
DEFS += -D__FIT_DISABLE_BFD
endif

release-shared:	LDFLAGS += -L$(SRCROOT)/lib/misc/${OUTPUT_DIR}
dbg-shared:	LDFLAGS += -L$(SRCROOT)/lib/misc/${OUTPUT_DIR_DBG}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L$(SRCROOT)/lib/misc/${OUTPUT_DIR_STLDBG}
endif

release-shared:	LDLIBS += -luuid -lmisc
dbg-shared:	LDLIBS += -luuid -lmiscg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS += -luuid -lmiscstlg
endif

LDLIBS += -ldl 
ifndef WITHOUT_BFD
LDLIBS += -lbfd
endif

check:	all-shared
	$(MAKE) -C ut $@

check-release-shared:	release-shared
	$(MAKE) -C ut $@

check-dbg-shared:	dbg-shared
	$(MAKE) -C ut $@

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C ut $@
endif

depend clean distclean mostlyclean maintainer-clean::
	${MAKE} -C ut $@

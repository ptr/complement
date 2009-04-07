# -*- Makefile -*- Time-stamp: <09/04/07 16:40:56 ptr>

SRCROOT := ../..

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I$(SRCROOT)/include
HEADERS_BASE = $(SRCROOT)/include/mt $(SRCROOT)/include/config $(SRCROOT)/include/misc

release-shared:	LDFLAGS += -L$(SRCROOT)/lib/misc/${OUTPUT_DIR}
dbg-shared:	LDFLAGS += -L$(SRCROOT)/lib/misc/${OUTPUT_DIR_DBG}
ifndef WITHOUT_STLPORT
stldbg-shared:	LDFLAGS += -L$(SRCROOT)/lib/misc/${OUTPUT_DIR_STLDBG}
endif

release-shared:	LDLIBS += -lmisc
dbg-shared:	LDLIBS += -lmiscg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS += -lmiscstlg
endif

LDLIBS += -ldl -lbfd

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

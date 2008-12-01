# -*- Makefile -*- Time-stamp: <08/12/02 00:04:29 ptr>

SRCROOT := ../../..

# EXTRA_POST := check
# EXTRA_POST_DBG := check-dbg
# EXTRA_POST_STLDBG := check-stldbg

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

ifndef WITHOUT_STLPORT
LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}
endif

ifndef WITHOUT_STLPORT
STLDTAG := stldbg-shared
endif

check:	release-shared dbg-shared ${STLDTAG}
	${PRG} || exit 1 && echo release ok
	${PRG_DBG} || exit 1 && echo dbg ok
ifndef WITHOUT_STLPORT
	${PRG_STLDBG} || exit 1 && echo stldbg ok
endif
	${test2_PRG} || exit 1 && echo release ok
	${test2_PRG_DBG} || exit 1 && echo dbg ok
ifndef WITHOUT_STLPORT
	${test2_PRG_STLDBG} || exit 1 && echo stldbg ok
endif
	${test3_PRG} || exit 1 && echo release ok
	${test3_PRG_DBG} || exit 1 && echo dbg ok
ifndef WITHOUT_STLPORT
	${test3_PRG_STLDBG} || exit 1 && echo stldbg ok
endif

PHONY += check

# -*- Makefile -*- Time-stamp: <08/12/02 00:31:28 ptr>

SRCROOT := ../../..

# EXTRA_POST := checks
# EXTRA_POST_DBG := checks-dbg
# EXTRA_POST_STLDBG := checks-stldbg

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

ifndef WITHOUT_STLPORT
LDFLAGS += -Wl,-rpath=${STLPORT_LIB_DIR}
endif

# Check that we really use C compiler here

define ldd_check
ldd $(1) | grep $(2) && exit 1
endef

ifndef WITHOUT_STLPORT
STLDTAG := stldbg-shared
endif

check:	release-shared dbg-shared ${STLDTAG}
	${PRG} || exit 1; ${call ldd_check,${PRG},stlport}; ${call ldd_check,${PRG},stdc++}; \
        echo release ok
	${PRG_DBG} || exit 1; ${call ldd_check,${PRG_DBG},stlport}; ${call ldd_check,${PRG_DBG},stdc++}; \
        echo dbg ok
ifndef WITHOUT_STLPORT
	${PRG_STLDBG} || exit 1; ${call ldd_check,${PRG_STLDBG},stlport}; ${call ldd_check,${PRG_DBG},stdc++}; \
        echo stldbg ok
endif

PHONY += check

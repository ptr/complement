# -*- Makefile -*- Time-stamp: <07/01/23 14:58:35 ptr>

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include

dbg-shared:	DEFS += -D__FIT_STEM_TRACE=1

ifdef WITHOUT_STLPORT
INCLUDES += -I${BOOST_INCLUDE_DIR}
else
stldbg-shared:	DEFS += -D__FIT_STEM_TRACE=1
endif

check:	install
	$(MAKE) -C ../../test/stem
	(cd ../../test/stem; ${OUTPUT_DIR}/stem_ut) || exit 1
	(cd ../../test/stem; ${OUTPUT_DIR_DBG}/stem_ut) || exit 1
	(cd ../../test/stem; ${OUTPUT_DIR_STLDBG}/stem_ut) || exit 1

check-release:	install-release-shared
	$(MAKE) -C ../../test/stem release-shared
	(cd ../../test/stem; ${OUTPUT_DIR}/stem_ut) || exit 1

check-dbg:	install-dbg-shared
	$(MAKE) -C ../../test/stem dbg-shared
	(cd ../../test/stem; ${OUTPUT_DIR_DBG}/stem_ut) || exit 1

ifndef WITHOUT_STLPORT
check-stldbg:	install-stldbg-shared
	$(MAKE) -C ../../test/stem stldbg-shared
	(cd ../../test/stem; ${OUTPUT_DIR_STLDBG}/stem_ut) || exit 1
endif

depend::
	$(MAKE) -C ../../test/stem depend

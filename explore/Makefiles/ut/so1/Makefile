# -*- Makefile -*- Time-stamp: <08/12/02 00:39:41 ptr>

SRCROOT := ../../..
# EXTRA_POST := checks

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I./include

ifndef WITHOUT_STLPORT
STLDTAG := stldbg-shared
endif

check:	release-shared dbg-shared ${STLDTAG}
	file ${SO_NAME_OUTxxx} | grep ELF > /dev/null || exit 1
	[ -h ${SO_NAME_OUTxx} ] || exit 1 && [ -h ${SO_NAME_OUTx} ] || exit 1 && [ -h ${SO_NAME_OUT} ] || exit 1
	file ${SO_NAME_OUT_DBGxxx} | grep ELF > /dev/null || exit 1
	[ -h ${SO_NAME_OUT_DBGxx} ] || exit 1 && [ -h ${SO_NAME_OUT_DBGx} ] || exit 1 && [ -h ${SO_NAME_OUT_DBG} ] || exit 1
ifndef WITHOUT_STLPORT
	file ${SO_NAME_OUT_STLDBGxxx} | grep ELF > /dev/null || exit 1
	[ -h ${SO_NAME_OUT_STLDBGxx} ] || exit 1 && [ -h ${SO_NAME_OUT_STLDBGx} ] || exit 1 && [ -h ${SO_NAME_OUT_STLDBG} ] || exit 1
endif

PHONY += check


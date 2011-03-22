# -*- Makefile -*- Time-stamp: <07/02/21 15:30:24 ptr>

SRCROOT := ../..
# CoMT_DIR := ../external/complement/explore

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${CoMT_INCLUDE_DIR} -I${BOOST_INCLUDE_DIR}
LDFLAGS += -L${INSTALL_LIB_DIR} -Wl,-rpath=${INSTALL_LIB_DIR}

release-shared: LDLIBS = -lxmt -lsockios -lNet -lstem -lboost_regex -lboost_filesystem -lboost_program_options
dbg-shared:	LDLIBS = -lxmtg -lsockiosg -lNetg -lstemg -lboost_regexg -lboost_filesystemg -lboost_program_optionsg
stldbg-shared:	LDLIBS = -lxmtstlg -lsockiosstlg -lNetstlg -lstemstlg -lboost_regexstlg  -lboost_filesystemstlg -lboost_program_optionsstlg

check-shared:	all-shared
	$(MAKE) -C unit check || exit 1

check-release-shared:	release-shared
	$(MAKE) -C unit check-release-shared || exit 1

check-dbg-shared:	dbg-shared
	$(MAKE) -C unit check-dbg-shared || exit 1

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	$(MAKE) -C unit check-stldbg-shared || exit 1
endif

depend::
	$(MAKE) -C unit depend

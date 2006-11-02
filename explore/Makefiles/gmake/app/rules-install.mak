# -*- makefile -*- Time-stamp: <06/11/02 10:45:53 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

ifndef WITHOUT_STLPORT
install:	install-release-shared install-dbg-shared install-stldbg-shared
else
install:	install-release-shared install-dbg-shared
endif

# The program name to be installed will be the same as compiled name,
# but it will be a bit altered in case of installation debug and/or
# stlport-debug program in the same catalog as 'release' program.

INSTALL_PRGNAME := ${PRGNAME}${EXE}

#ifeq (${INSTALL_BIN_DIR},${INSTALL_BIN_DIR_DBG})
#INSTALL_PRGNAME_DBG := ${PRGNAME}g${EXE}
#else
INSTALL_PRGNAME_DBG := ${INSTALL_PRGNAME}
#endif

ifndef WITHOUT_STLPORT
#ifeq (${INSTALL_BIN_DIR},${INSTALL_BIN_DIR_STLDBG})
#INSTALL_PRGNAME_STLDBG := ${PRGNAME}stlg${EXE}
#else
INSTALL_PRGNAME_STLDBG := ${INSTALL_PRGNAME}
#endif
endif

#ifeq (${INSTALL_BIN_DIR_DBG},${INSTALL_BIN_DIR_STLDBG})
#INSTALL_PRGNAME_DBG := ${PRGNAME}g${EXE}
#INSTALL_PRGNAME_STLDBG := ${PRGNAME}stlg${EXE}
#endif

install-release-shared: release-shared $(INSTALL_BIN_DIR)
	$(INSTALL_EXE) ${PRG} $(INSTALL_BIN_DIR)/${INSTALL_PRGNAME}

install-dbg-shared: dbg-shared $(INSTALL_BIN_DIR_DBG)
	$(INSTALL_EXE) ${PRG_DBG} $(INSTALL_BIN_DIR_DBG)/${INSTALL_PRGNAME_DBG}

ifndef WITHOUT_STLPORT
install-stldbg-shared: stldbg-shared $(INSTALL_BIN_DIR_STLDBG)
	$(INSTALL_EXE) ${PRG_STLDBG} $(INSTALL_BIN_DIR_STLDBG)/${INSTALL_PRGNAME_STLDBG}
endif

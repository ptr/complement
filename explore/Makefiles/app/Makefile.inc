# Time-stamp: <01/11/09 12:54:29 ptr>
# $Id$

all:	all-shared all-static

all-shared:	release-shared dbg-shared stldbg-shared

all-static:	release-static dbg-static stldbg-static

all-debug:	dbg-shared stldbg-shared dbg-static stldbg-static

all-release:	release-shared release-static

include $(BASEDIR)/Makefiles/Makefile.inc.ver
include $(BASEDIR)/Makefiles/Makefile.inc
-include $(BASEDIR)/Makefiles/Makefile.dep.${COMPILER_NAME}
include ${BASEDIR}/Makefiles/Makefile.inc.${COMPILER_NAME}
include ${BASEDIR}/Makefiles/app/Makefile.inc.${COMPILER_NAME}

PRG     = $(OUTPUT_DIR)/${PRGNAME}
PRG_A   = $(OUTPUT_DIR_A)/${PRGNAME}
PRG_DBG = $(OUTPUT_DIR_DBG)/${PRGNAME}
PRG_STLDBG = $(OUTPUT_DIR_STLDBG)/${PRGNAME}
PRG_A_DBG = $(OUTPUT_DIR_A_DBG)/${PRGNAME}
PRG_A_STLDBG = $(OUTPUT_DIR_A_STLDBG)/${PRGNAME}

dbg-shared:	$(OUTPUT_DIR_DBG) ${PRG_DBG}

stldbg-shared:	$(OUTPUT_DIR_STLDBG) ${PRG_STLDBG}

dbg-static:	$(OUTPUT_DIR_A_DBG) ${PRG_A_DBG}

stldbg-static:	$(OUTPUT_DIR_A_STLDBG) ${PRG_A_STLDBG}

dbg-shared-dep:	$(OUTPUT_DIR_DBG) $(DEP_DBG)

stldbg-shared-dep:	$(OUTPUT_DIR_STLDBG) $(DEP_STLDBG)

dbg-static-dep:	$(OUTPUT_DIR_A_DBG) $(DEP_A_DBG)

stldbg-static-dep:	$(OUTPUT_DIR_A_STLDBG) $(DEP_A_STLDBG)

release-shared:	$(OUTPUT_DIR) ${PRG}

release-static:	$(OUTPUT_DIR) ${PRG_A}

release-shared-dep:	$(OUTPUT_DIR) $(DEP)

release-static-dep:	$(OUTPUT_DIR_A) $(DEP_A)

${PRG}:	$(OBJ) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ) $(LDLIBS)

${PRG_DBG}:	$(OBJ_DBG) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_DBG) $(LDLIBS)

${PRG_STLDBG}:	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_STLDBG) $(LDLIBS)

${PRG_A}:	$(OBJ_A) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_A) $(LDLIBS)

${PRG_A_DBG}:	$(OBJ_A_DBG) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_A_DBG) $(LDLIBS)

${PRG_A_STLDBG}:	$(OBJ_A_STLDBG) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_A_STLDBG) $(LDLIBS)


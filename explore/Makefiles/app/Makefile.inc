# -*- makefile -*- Time-stamp: <02/09/03 19:14:11 ptr>
# $Id$

include $(BASEDIR)/Makefiles/Makefile.inc.ver
include $(BASEDIR)/Makefiles/Makefile.inc
# -include $(BASEDIR)/Makefiles/Makefile.dep.${COMPILER_NAME}
include ${BASEDIR}/Makefiles/Makefile.inc.${COMPILER_NAME}
include ${BASEDIR}/Makefiles/app/Makefile.inc.${COMPILER_NAME}

PRG        := $(OUTPUT_DIR)/${PRGNAME}
PRG_DBG    := $(OUTPUT_DIR_DBG)/${PRGNAME}
PRG_STLDBG := $(OUTPUT_DIR_STLDBG)/${PRGNAME}

dbg-shared:	$(OUTPUT_DIR_DBG) ${PRG_DBG}

stldbg-shared:	$(OUTPUT_DIR_STLDBG) ${PRG_STLDBG}

release-shared:	$(OUTPUT_DIR) ${PRG}

dbg-static:	$(OUTPUT_DIR_DBG) ${PRG_DBG}

stldbg-static:	$(OUTPUT_DIR_STLDBG) ${PRG_STLDBG}

release-static:	$(OUTPUT_DIR) ${PRG}

${PRG}:	$(OBJ) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ) $(LDLIBS)

${PRG_DBG}:	$(OBJ_DBG) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_DBG) $(LDLIBS)

${PRG_STLDBG}:	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_STLDBG) $(LDLIBS)


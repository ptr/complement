# -*- makefile -*- Time-stamp: <06/11/10 17:10:45 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

dbg-shared:	$(EXTRA_PRE_DBG) $(OUTPUT_DIR_DBG) ${PRG_DBG} $(EXTRA_POST_DBG)

dbg-static:	$(EXTRA_PRE_DBG) $(OUTPUT_DIR_DBG) ${PRG_DBG} $(EXTRA_POST_DBG)

release-shared:	$(EXTRA_PRE) $(OUTPUT_DIR) ${PRG} $(EXTRA_POST)

release-static:	$(EXTRA_PRE) $(OUTPUT_DIR) ${PRG} $(EXTRA_POST)

ifndef WITHOUT_STLPORT
stldbg-shared:	$(EXTRA_PRE_STLDBG) $(OUTPUT_DIR_STLDBG) ${PRG_STLDBG} $(EXTRA_POST_STLDBG)

stldbg-static:	$(EXTRA_PRE_STLDBG) $(OUTPUT_DIR_STLDBG) ${PRG_STLDBG} $(EXTRA_POST_STLDBG)
endif

ifeq ("${_C_SOURCES_ONLY}","")

${PRG}:	$(OBJ) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) ${START_OBJ} $(OBJ) $(LDLIBS) ${STDLIBS} ${END_OBJ}

${PRG_DBG}:	$(OBJ_DBG) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) ${START_OBJ} $(OBJ_DBG) $(LDLIBS) ${STDLIBS} ${END_OBJ}

ifndef WITHOUT_STLPORT
${PRG_STLDBG}:	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) ${START_OBJ} $(OBJ_STLDBG) $(LDLIBS) ${STDLIBS} ${END_OBJ}
endif

else

${PRG}:	$(OBJ) $(LIBSDEP)
	$(LINK.c) $(LINK_OUTPUT_OPTION) $(OBJ) $(LDLIBS)

${PRG_DBG}:	$(OBJ_DBG) $(LIBSDEP)
	$(LINK.c) $(LINK_OUTPUT_OPTION) $(OBJ_DBG) $(LDLIBS)

ifndef WITHOUT_STLPORT
${PRG_STLDBG}:	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK.c) $(LINK_OUTPUT_OPTION) $(OBJ_STLDBG) $(LDLIBS)
endif

endif

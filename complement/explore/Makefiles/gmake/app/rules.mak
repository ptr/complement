# -*- makefile -*- Time-stamp: <06/11/17 10:34:26 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

dbg-shared:	$(EXTRA_PRE_DBG) $(OUTPUT_DIR_DBG) ${PRG_DBG} ${ALLPRGS_DBG} $(EXTRA_POST_DBG)

dbg-static:	$(EXTRA_PRE_DBG) $(OUTPUT_DIR_DBG) ${PRG_DBG} ${ALLPRGS_DBG} $(EXTRA_POST_DBG)

release-shared:	$(EXTRA_PRE) $(OUTPUT_DIR) ${PRG} ${ALLPRGS} $(EXTRA_POST)

release-static:	$(EXTRA_PRE) $(OUTPUT_DIR) ${PRG} ${ALLPRGS} $(EXTRA_POST)

ifndef WITHOUT_STLPORT
stldbg-shared:	$(EXTRA_PRE_STLDBG) $(OUTPUT_DIR_STLDBG) ${PRG_STLDBG} ${ALLPRGS_STLDBG} $(EXTRA_POST_STLDBG)

stldbg-static:	$(EXTRA_PRE_STLDBG) $(OUTPUT_DIR_STLDBG) ${PRG_STLDBG} ${ALLPRGS_STLDBG} $(EXTRA_POST_STLDBG)
endif

define prog_lnk
ifeq ($${_$(1)_C_SOURCES_ONLY},)
$${$(1)_PRG}:	$$($(1)_OBJ) $$(LIBSDEP)
	$$(LINK.cc) $$(LINK_OUTPUT_OPTION) $${START_OBJ} $$($(1)_OBJ) $$(LDLIBS) $${STDLIBS} $${END_OBJ}

$${$(1)_PRG_DBG}:	$$($(1)_OBJ_DBG) $$(LIBSDEP)
	$$(LINK.cc) $$(LINK_OUTPUT_OPTION) $${START_OBJ} $$($(1)_OBJ_DBG) $$(LDLIBS) $${STDLIBS} $${END_OBJ}

ifndef WITHOUT_STLPORT
$${$(1)_PRG_STLDBG}:	$$($(1)_OBJ_STLDBG) $$(LIBSDEP)
	$$(LINK.cc) $$(LINK_OUTPUT_OPTION) $${START_OBJ} $$($(1)_OBJ_STLDBG) $$(LDLIBS) $${STDLIBS} $${END_OBJ}
endif
else
$${$(1)_PRG}:	$$($(1)_OBJ) $$(LIBSDEP)
	$$(LINK.c) $$(LINK_OUTPUT_OPTION) $$($(1)_OBJ) $$(LDLIBS)

$${$(1)_PRG_DBG}:	$$(OBJ_DBG) $$(LIBSDEP)
	$$(LINK.c) $$(LINK_OUTPUT_OPTION) $$($(1)_OBJ_DBG) $$(LDLIBS)

ifndef WITHOUT_STLPORT
$${$(1)_PRG_STLDBG}:	$$($(1)_OBJ_STLDBG) $$(LIBSDEP)
	$$(LINK.c) $$(LINK_OUTPUT_OPTION) $$($(1)_OBJ_STLDBG) $$(LDLIBS)
endif
endif
endef

$(foreach prg,$(PRGNAMES),$(eval $(call prog_lnk,$(prg))))

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

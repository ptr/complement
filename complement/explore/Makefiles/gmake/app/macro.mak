# Time-stamp: <06/11/16 23:37:38 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

ALLPRGS = 
ALLPRGS_DBG =
ALLPRGS_STLDBG =

define prog_prog
$(1)_PRG        := $(OUTPUT_DIR)/$(1)${EXE}
$(1)_PRG_DBG    := $(OUTPUT_DIR_DBG)/$(1)${EXE}
$(1)_PRG_STLDBG := $(OUTPUT_DIR_STLDBG)/$(1)${EXE}

ALLPRGS        += $${$(1)_PRG}
ALLPRGS_DBG    += $${$(1)_PRG_DBG}
ALLPRGS_STLDBG += $${$(1)_PRG_STLDBG}
endef

$(foreach prg,$(PRGNAMES),$(eval $(call prog_prog,$(prg))))

PRG        := $(OUTPUT_DIR)/${PRGNAME}${EXE}
PRG_DBG    := $(OUTPUT_DIR_DBG)/${PRGNAME}${EXE}
PRG_STLDBG := $(OUTPUT_DIR_STLDBG)/${PRGNAME}${EXE}

LDFLAGS += ${LDSEARCH}

# -*- Makefile -*- Time-stamp: <01/07/27 14:26:33 ptr>
# $Id$

DBG_SUFFIX = -g

OUTPUT_DIR     = obj/$(COMPILER_NAME)/shared$(EXTRA_DIRS)
OUTPUT_DIR_DBG = obj/$(COMPILER_NAME)/shared-g$(EXTRA_DIRS)
OUTPUT_DIR_STLDBG = obj/$(COMPILER_NAME)/shared-stlg$(EXTRA_DIRS)

OUTPUT_DIR_A     = obj/$(COMPILER_NAME)/static$(EXTRA_DIRS)
OUTPUT_DIR_A_DBG = obj/$(COMPILER_NAME)/static-g$(EXTRA_DIRS)
OUTPUT_DIR_A_STLDBG = obj/$(COMPILER_NAME)/static-stlg$(EXTRA_DIRS)

INSTALL_LIB_DIR = $$HOME/lib

$(OUTPUT_DIR):
	@if [ ! -d ${OUTPUT_DIR} ] ; then \
	  mkdir -p $(OUTPUT_DIR) ; \
	fi

$(OUTPUT_DIR_DBG):
	@if [ ! -d ${OUTPUT_DIR_DBG} ] ; then \
	  mkdir -p $(OUTPUT_DIR_DBG) ; \
	fi

$(OUTPUT_DIR_STLDBG):
	@if [ ! -d ${OUTPUT_DIR_STLDBG} ] ; then \
	  mkdir -p $(OUTPUT_DIR_STLDBG) ; \
	fi

$(OUTPUT_DIR_A):
	@if [ ! -d ${OUTPUT_DIR_A} ] ; then \
	  mkdir -p $(OUTPUT_DIR_A) ; \
	fi

$(OUTPUT_DIR_A_DBG):
	@if [ ! -d ${OUTPUT_DIR_A_DBG} ] ; then \
	  mkdir -p $(OUTPUT_DIR_A_DBG) ; \
	fi

$(OUTPUT_DIR_A_STLDBG):
	@if [ ! -d ${OUTPUT_DIR_A_STLDBG} ] ; then \
	  mkdir -p $(OUTPUT_DIR_A_STLDBG) ; \
	fi

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR)/%.o:	%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_DBG)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_DBG)/%.o:	%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_STLDBG)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_STLDBG)/%.o:	%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_A)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_A)/%.o:	%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_A_DBG)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_A_DBG)/%.o:	%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_A_STLDBG)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_A_STLDBG)/%.o:	%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

OBJ      = $(SRC_CC:%.cc=$(OUTPUT_DIR)/%.o)
OBJ     += $(SRC_C:%.c=$(OUTPUT_DIR)/%.o)
OBJ_DBG  = $(SRC_CC:%.cc=$(OUTPUT_DIR_DBG)/%.o)
OBJ_DBG += $(SRC_C:%.c=$(OUTPUT_DIR_DBG)/%.o)
OBJ_STLDBG  = $(SRC_CC:%.cc=$(OUTPUT_DIR_STLDBG)/%.o)
OBJ_STLDBG += $(SRC_C:%.c=$(OUTPUT_DIR_STLDBG)/%.o)
DEP      = $(SRC_CC:%.cc=$(OUTPUT_DIR)/%.d)
DEP     += $(SRC_C:%.c=$(OUTPUT_DIR)/%.d)
DEP_DBG  = $(SRC_CC:%.cc=$(OUTPUT_DIR_DBG)/%.d)
DEP_DBG += $(SRC_C:%.c=$(OUTPUT_DIR_DBG)/%.d)
DEP_STLDBG  = $(SRC_CC:%.cc=$(OUTPUT_DIR_STLDBG)/%.d)
DEP_STLDBG += $(SRC_C:%.c=$(OUTPUT_DIR_STLDBG)/%.d)

OBJ_A    = $(SRC_CC:%.cc=$(OUTPUT_DIR_A)/%.o)
OBJ_A   += $(SRC_C:%.c=$(OUTPUT_DIR_A)/%.o)
OBJ_A_DBG  = $(SRC_CC:%.cc=$(OUTPUT_DIR_A_DBG)/%.o)
OBJ_A_DBG += $(SRC_C:%.c=$(OUTPUT_DIR_A_DBG)/%.o)
OBJ_A_STLDBG  = $(SRC_CC:%.cc=$(OUTPUT_DIR_A_STLDBG)/%.o)
OBJ_A_STLDBG += $(SRC_C:%.c=$(OUTPUT_DIR_A_STLDBG)/%.o)
DEP_A    = $(SRC_CC:%.cc=$(OUTPUT_DIR_A)/%.d)
DEP_A   += $(SRC_C:%.c=$(OUTPUT_DIR_A)/%.d)
DEP_A_DBG  = $(SRC_CC:%.cc=$(OUTPUT_DIR_A_DBG)/%.d)
DEP_A_DBG += $(SRC_C:%.c=$(OUTPUT_DIR_A_DBG)/%.d)
DEP_A_STLDBG  = $(SRC_CC:%.cc=$(OUTPUT_DIR_A_STLDBG)/%.d)
DEP_A_STLDBG += $(SRC_C:%.c=$(OUTPUT_DIR_A_STLDBG)/%.d)

clear-release-shared::
	@-rm -f $(OBJ) $(DEP)

clear-release-static::
	@-rm -f $(OBJ_A) $(DEP_A)

clear-dbg-shared::
	@-rm -f $(OBJ_DBG) $(DEP_DBG)

clear-stldbg-shared::
	@-rm -f $(OBJ_STLDBG) $(DEP_STLDBG)

clear-dbg-static::
	@-rm -f $(OBJ_A_DBG) $(DEP_A_DBG)

clear-stldbg-static::
	@-rm -f $(OBJ_A_STLDBG) $(DEP_A_STLDBG)

clear::	clear-release-shared \
	clear-dbg-shared clear-stldbg-shared \
	clear-release-static \
	clear-dbg-static clear-stldbg-static
	rm -f .make.state core .make.depend

# %.o:	../%.cc
#	$(COMPILE.cc) $(OUTPUT_OPTION) $<
#
#%.o:	../%.c
#	$(COMPILE.c) $(OUTPUT_OPTION) $<


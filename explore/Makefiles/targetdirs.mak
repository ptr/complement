# Time-stamp: <03/07/03 17:34:03 ptr>
# $Id$

OUTPUT_DIR     := obj/$(COMPILER_NAME)/shared$(EXTRA_DIRS)
OUTPUT_DIR_DBG := obj/$(COMPILER_NAME)/shared-g$(EXTRA_DIRS)
OUTPUT_DIR_STLDBG := obj/$(COMPILER_NAME)/shared-stlg$(EXTRA_DIRS)

# I use the same catalog, as for shared:
OUTPUT_DIR_A := $(OUTPUT_DIR)
OUTPUT_DIR_A_DBG := $(OUTPUT_DIR_DBG)
OUTPUT_DIR_A_STLDBG := $(OUTPUT_DIR_STLDBG)

INSTALL_LIB_DIR := ${BASEDIR}/build/lib
INSTALL_LIB_DIR_DBG := ${BASEDIR}/build/lib-g
INSTALL_LIB_DIR_STLDBG := ${BASEDIR}/build/lib_stl-g
INSTALL_BIN_DIR := ${BASEDIR}/build/bin
INSTALL_BIN_DIR_DBG := ${BASEDIR}/build/bin-g
INSTALL_BIN_DIR_STLDBG := ${BASEDIR}/build/bin_stl-g

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

# if ${OUTPUT_DIR_A} != ${OUTPUT_DIR} and so on
# provide mkdir here:
#  ...

ifneq ($(OUTPUT_DIR_A),$(OUTPUT_DIR))
$(OUTPUT_DIR_A):
	@if [ ! -d ${OUTPUT_DIR_A} ] ; then \
	  mkdir -p $(OUTPUT_DIR_A) ; \
	fi
endif

ifneq ($(OUTPUT_DIR_A_DBG),$(OUTPUT_DIR_DBG))
$(OUTPUT_DIR_DBG):
	@if [ ! -d ${OUTPUT_DIR_DBG} ] ; then \
	  mkdir -p $(OUTPUT_DIR_DBG) ; \
	fi
endif

ifneq ($(OUTPUT_DIR_A_STLDBG),$(OUTPUT_DIR_STLDBG))
$(OUTPUT_DIR_STLDBG):
	@if [ ! -d ${OUTPUT_DIR_STLDBG} ] ; then \
	  mkdir -p $(OUTPUT_DIR_STLDBG) ; \
	fi
endif

# Time-stamp: <03/07/15 15:23:19 ptr>
# $Id$

OUTPUT_DIR             := obj/$(COMPILER_NAME)/shared$(EXTRA_DIRS)
OUTPUT_DIR_DBG         := obj/$(COMPILER_NAME)/shared-g$(EXTRA_DIRS)
OUTPUT_DIR_STLDBG      := obj/$(COMPILER_NAME)/shared-stlg$(EXTRA_DIRS)

# file to store generated dependencies for make:
DEPENDS_COLLECTION     := obj/$(COMPILER_NAME)/.make.depend

# I use the same catalog, as for shared:
OUTPUT_DIR_A           := $(OUTPUT_DIR)
OUTPUT_DIR_A_DBG       := $(OUTPUT_DIR_DBG)
OUTPUT_DIR_A_STLDBG    := $(OUTPUT_DIR_STLDBG)

INSTALL_LIB_DIR        := ${SRCROOT}/build/lib
INSTALL_LIB_DIR_DBG    := ${SRCROOT}/build/lib-g
INSTALL_LIB_DIR_STLDBG := ${SRCROOT}/build/lib_stl-g
INSTALL_BIN_DIR        := ${SRCROOT}/build/bin
INSTALL_BIN_DIR_DBG    := ${SRCROOT}/build/bin-g
INSTALL_BIN_DIR_STLDBG := ${SRCROOT}/build/bin_stl-g

OUTPUT_DIRS := $(OUTPUT_DIR) $(OUTPUT_DIR_DBG) $(OUTPUT_DIR_STLDBG) \
               $(OUTPUT_DIR_A) $(OUTPUT_DIR_A_DBG) $(OUTPUT_DIR_A_STLDBG)

# sort will remove duplicates:
OUTPUT_DIRS := $(sort $(OUTPUT_DIRS))

PHONY += dirs $(OUTPUT_DIRS)

dirs:	$(OUTPUT_DIRS)

$(OUTPUT_DIRS):
	@if [ ! -d $@ ] ; then \
	  mkdir -p $@ ; \
	fi


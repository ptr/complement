# Time-stamp: <06/10/12 19:53:42 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

ifdef TARGET_OS
TARGET_NAME := ${TARGET_OS}-
else
TARGET_NAME :=
endif

BASE_OUTPUT_DIR        := obj
PRE_OUTPUT_DIR         := $(BASE_OUTPUT_DIR)/$(TARGET_NAME)$(COMPILER_NAME)
OUTPUT_DIR             := $(PRE_OUTPUT_DIR)/so$(EXTRA_DIRS)
OUTPUT_DIR_DBG         := $(PRE_OUTPUT_DIR)/so_g$(EXTRA_DIRS)
OUTPUT_DIR_STLDBG      := $(PRE_OUTPUT_DIR)/so_stlg$(EXTRA_DIRS)

# file to store generated dependencies for make:
DEPENDS_COLLECTION     := $(PRE_OUTPUT_DIR)/.make.depend

# I use the same catalog, as for shared:
OUTPUT_DIR_A           := $(OUTPUT_DIR)
OUTPUT_DIR_A_DBG       := $(OUTPUT_DIR_DBG)
OUTPUT_DIR_A_STLDBG    := $(OUTPUT_DIR_STLDBG)

BASE_INSTALL_DIR       ?= ${SRCROOT}/build/$(TARGET_NAME)

BASE_INSTALL_LIB_DIR   ?= ${BASE_INSTALL_DIR}
BASE_INSTALL_BIN_DIR   ?= ${BASE_INSTALL_DIR}

INSTALL_LIB_DIR        ?= ${BASE_INSTALL_LIB_DIR}lib
INSTALL_LIB_DIR_DBG    ?= ${BASE_INSTALL_LIB_DIR}lib
INSTALL_LIB_DIR_STLDBG ?= ${BASE_INSTALL_LIB_DIR}lib
INSTALL_BIN_DIR        ?= ${BASE_INSTALL_BIN_DIR}bin
INSTALL_BIN_DIR_DBG    ?= ${INSTALL_BIN_DIR}_g
INSTALL_BIN_DIR_STLDBG ?= ${INSTALL_BIN_DIR}_stlg

OUTPUT_DIRS := $(OUTPUT_DIR) $(OUTPUT_DIR_DBG) $(OUTPUT_DIR_STLDBG) \
               $(OUTPUT_DIR_A) $(OUTPUT_DIR_A_DBG) $(OUTPUT_DIR_A_STLDBG)

INSTALL_LIB_DIRS := $(INSTALL_LIB_DIR) $(INSTALL_LIB_DIR_DBG) $(INSTALL_LIB_DIR_STLDBG)
INSTALL_BIN_DIRS := $(INSTALL_BIN_DIR) $(INSTALL_BIN_DIR_DBG) $(INSTALL_BIN_DIR_STLDBG)

# sort will remove duplicates:
OUTPUT_DIRS := $(sort $(OUTPUT_DIRS))
INSTALL_LIB_DIRS := $(sort $(INSTALL_LIB_DIRS))
INSTALL_BIN_DIRS := $(sort $(INSTALL_BIN_DIRS))
INSTALL_DIRS := $(sort $(INSTALL_LIB_DIRS) $(INSTALL_BIN_DIRS))

PHONY += $(OUTPUT_DIRS) $(INSTALL_DIRS)

define createdirs
@for d in $@ ; do \
  if [ -e $$d -a -f $$d ] ; then \
    echo "ERROR: Regular file $$d present, directory instead expected" ; \
    exit 1; \
  elif [ ! -d $$d ] ; then \
    mkdir -p $$d ; \
  fi ; \
done
endef

$(OUTPUT_DIRS):
	$(createdirs)

$(INSTALL_DIRS):
	$(createdirs)


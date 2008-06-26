# -*- makefile -*- Time-stamp: <07/05/31 00:12:45 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005-2007
# Petr Ovtchenkov
#
# Copyright (c) 2006, 2007
# Francois Dumont
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

INSTALL_TAGS ?= install-shared

PHONY += install $(INSTALL_TAGS)

install:	$(INSTALL_TAGS)

install-release-shared: release-shared $(INSTALL_BIN_DIR) $(INSTALL_LIB_DIR)
	$(INSTALL_SO) $(subst /,\,$(SO_NAME_OUT)) $(subst /,\,$(INSTALL_BIN_DIR)/)
	$(INSTALL_SO) $(subst /,\,$(LIB_NAME_OUT)) $(subst /,\,$(INSTALL_LIB_DIR)/)

install-dbg-shared: dbg-shared $(INSTALL_BIN_DIR) $(INSTALL_LIB_DIR_DBG)
	$(INSTALL_SO) $(subst /,\,$(SO_NAME_OUT_DBG)) $(subst /,\,$(INSTALL_BIN_DIR)/)
	$(INSTALL_SO) $(subst /,\,$(LIB_NAME_OUT_DBG)) $(subst /,\,$(INSTALL_LIB_DIR_DBG)/)

install-stldbg-shared: stldbg-shared $(INSTALL_BIN_DIR) $(INSTALL_LIB_DIR_STLDBG)
	$(INSTALL_SO) $(subst /,\,$(SO_NAME_OUT_STLDBG)) $(subst /,\,$(INSTALL_BIN_DIR)/)
	$(INSTALL_SO) $(subst /,\,$(LIB_NAME_OUT_STLDBG)) $(subst /,\,$(INSTALL_LIB_DIR_STLDBG)/)


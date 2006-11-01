# -*- makefile -*- Time-stamp: <03/07/15 18:26:22 ptr>

INSTALL_TAGS ?= install-release-shared install-dbg-shared install-stldbg-shared

PHONY += install $(INSTALL_TAGS)

install:	$(INSTALL_TAGS)

install-static: all-static install-release-static install-dbg-static install-stldbg-static
install-shared: all-shared install-release-shared install-dbg-shared install-stldbg-shared

install-release-shared:	release-shared $(INSTALL_BIN_DIR) $(INSTALL_LIB_DIR)
	@if not exist $(subst /,\,$(INSTALL_BIN_DIR)/) mkdir $(subst /,\,$(INSTALL_BIN_DIR)/)
	@if not exist $(subst /,\,$(INSTALL_LIB_DIR)/) mkdir $(subst /,\,$(INSTALL_LIB_DIR)/)
	$(INSTALL_SO) $(subst /,\,$(SO_NAME_OUT)) $(subst /,\,$(INSTALL_BIN_DIR)/)
	$(INSTALL_SO) $(subst /,\,$(LIB_NAME_OUT)) $(subst /,\,$(INSTALL_LIB_DIR)/)

install-dbg-shared:	dbg-shared $(INSTALL_BIN_DIR) $(INSTALL_LIB_DIR_DBG)
	@if not exist $(subst /,\,$(INSTALL_BIN_DIR)/) mkdir $(subst /,\,$(INSTALL_BIN_DIR)/)
	@if not exist $(subst /,\,$(INSTALL_LIB_DIR_DBG)/) mkdir $(subst /,\,$(INSTALL_LIB_DIR_DBG)/)
	$(INSTALL_SO) $(subst /,\,$(SO_NAME_OUT_DBG)) $(subst /,\,$(INSTALL_BIN_DIR)/)
	$(INSTALL_SO) $(subst /,\,$(LIB_NAME_OUT_DBG)) $(subst /,\,$(INSTALL_LIB_DIR_DBG)/)

install-stldbg-shared:	stldbg-shared $(INSTALL_BIN_DIR) $(INSTALL_LIB_DIR_STLDBG)
	@if not exist $(subst /,\,$(INSTALL_BIN_DIR)/) mkdir $(subst /,\,$(INSTALL_BIN_DIR)/)
	@if not exist $(subst /,\,$(INSTALL_LIB_DIR_STLDBG)/) mkdir $(subst /,\,$(INSTALL_LIB_DIR_STLDBG)/)
	$(INSTALL_SO) $(subst /,\,$(SO_NAME_OUT_STLDBG)) $(subst /,\,$(INSTALL_BIN_DIR)/)
	$(INSTALL_SO) $(subst /,\,$(LIB_NAME_OUT_STLDBG)) $(subst /,\,$(INSTALL_LIB_DIR_STLDBG)/)


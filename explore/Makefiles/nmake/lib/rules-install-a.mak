# Time-stamp: <03/09/28 19:16:37 ptr>
# $Id$

install-release-static: release-static
	$(INSTALL_A) $(A_NAME_OUT) $(INSTALL_LIB_DIR)

install-dbg-static: dbg-static
	$(INSTALL_A) $(A_NAME_OUT_DBG) $(INSTALL_LIB_DIR_DBG)

install-stldbg-static: stldbg-static
	$(INSTALL_A) $(A_NAME_OUT_STLDBG) $(INSTALL_LIB_DIR_STLDBG)

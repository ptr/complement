# Time-stamp: <03/09/28 19:16:37 ptr>
# $Id$

!ifndef INSTALL_TAGS
INSTALL_TAGS= install-release-shared install-dbg-shared install-stldbg-shared
!endif

install:	$(INSTALL_TAGS)

install-release-shared: release-shared
	$(INSTALL_SO) $(SO_NAME_OUT) $(INSTALL_LIB_DIR)
	$(INSTALL_SO) $(LIB_NAME_OUT) $(INSTALL_LIB_DIR)
	$(INSTALL_SO) $(EXP_NAME_OUT) $(INSTALL_LIB_DIR)

install-dbg-shared: dbg-shared
	$(INSTALL_SO) $(SO_NAME_OUT_DBG) $(INSTALL_LIB_DIR_DBG)
	$(INSTALL_SO) $(LIB_NAME_OUT_DBG) $(INSTALL_LIB_DIR_DBG)
	$(INSTALL_SO) $(EXP_NAME_OUT_DBG) $(INSTALL_LIB_DIR_DBG)

install-stldbg-shared: stldbg-shared
	$(INSTALL_SO) $(SO_NAME_OUT_STLDBG) $(INSTALL_LIB_DIR_STLDBG)
	$(INSTALL_SO) $(LIB_NAME_OUT_STLDBG) $(INSTALL_LIB_DIR_STLDBG)
	$(INSTALL_SO) $(EXP_NAME_OUT_STLDBG) $(INSTALL_LIB_DIR_STLDBG)

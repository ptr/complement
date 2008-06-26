# Time-stamp: <03/10/26 16:42:14 ptr>
# $Id$

!ifndef INSTALL_TAGS
INSTALL_TAGS= install-release-shared install-dbg-shared install-stldbg-shared install-release-static install-dbg-static install-stldbg-static
!endif

install:	$(INSTALL_TAGS)

install-shared: all-shared install-release-shared install-dbg-shared install-stldbg-shared

install-release-shared: release-shared $(INSTALL_BIN_DIR)
	$(INSTALL_SO) $(PRG) $(INSTALL_BIN_DIR)

install-dbg-shared: dbg-shared $(INSTALL_BIN_DIR_DBG)
	$(INSTALL_SO) $(PRG_DBG) $(INSTALL_BIN_DIR_DBG)

install-stldbg-shared: stldbg-shared $(INSTALL_BIN_DIR_STLDBG)
	$(INSTALL_SO) $(PRG_STLDBG) $(INSTALL_BIN_DIR_STLDBG)

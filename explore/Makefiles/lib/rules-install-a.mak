# -*- makefile -*- Time-stamp: <03/06/22 20:09:51 ptr>
# $Id$

install-release-static:	release-static
	$(INSTALL_A) ${A_NAME_SHORT} $(INSTALL_LIB_DIR)

install-dbg-static:	dbg-static
	$(INSTALL_A) ${A_NAME_SHORT_DBG} $(INSTALL_LIB_DIR_DBG)

install-stldbg-static:	stldbg-static
	$(INSTALL_A) ${A_NAME_SHORT_STLDBG} $(INSTALL_LIB_DIR_STLDBG)

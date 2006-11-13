# -*- makefile -*- Time-stamp: <06/11/11 00:43:13 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

clean::
	@-rm -f ${PRG}
	@-rm -f ${PRG_DBG}
	@-rm -f ${PRG_STLDBG}

distclean::
	@-rm -f $(DEPENDS_COLLECTION)
	@-rmdir -p ${OUTPUT_DIR} ${OUTPUT_DIR_DBG} ${OUTPUT_DIR_STLDBG} 2>/dev/null || exit 0

uninstall:
	@-rm -f $(INSTALL_BIN_DIR)/$(PRG)
	@-rm -f $(INSTALL_BIN_DIR_DBG)/$(PRG_DBG)
	@-rm -f $(INSTALL_BIN_DIR_STLDBG)/$(PRG_STLDBG)

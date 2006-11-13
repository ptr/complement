# -*- makefile -*- Time-stamp: <06/11/11 00:43:08 ptr>
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
	@-rm -f ${SO_NAME_OUT}
	@-rm -f ${SO_NAME_OUTx}
	@-rm -f ${SO_NAME_OUTxx}
	@-rm -f ${SO_NAME_OUTxxx}
	@-rm -f ${SO_NAME_OUT_DBG}
	@-rm -f ${SO_NAME_OUT_DBGx}
	@-rm -f ${SO_NAME_OUT_DBGxx}
	@-rm -f ${SO_NAME_OUT_DBGxxx}
	@-rm -f ${SO_NAME_OUT_STLDBG}
	@-rm -f ${SO_NAME_OUT_STLDBGx}
	@-rm -f ${SO_NAME_OUT_STLDBGxx}
	@-rm -f ${SO_NAME_OUT_STLDBGxxx}
	@-rm -f ${A_NAME_OUT}
	@-rm -f ${A_NAME_OUT_DBG}
	@-rm -f ${A_NAME_OUT_STLDBG}
ifeq ($(OSNAME), cygming)
	@-rm -f ${LIB_NAME_OUT}
	@-rm -f ${LIB_NAME_OUT_DBG}
	@-rm -f ${LIB_NAME_OUT_STLDBG}
	@-rm -f ${RES}
	@-rm -f ${RES_DBG}
	@-rm -f ${RES_STLDBG}
endif

distclean::
	@-rm -f $(DEPENDS_COLLECTION)
	@-rmdir -p ${OUTPUT_DIR} ${OUTPUT_DIR_DBG} ${OUTPUT_DIR_STLDBG} 2>/dev/null || exit 0

uninstall:
	@-rm -f $(INSTALL_LIB_DIR)/$(SO_NAME)
	@-rm -f $(INSTALL_LIB_DIR)/$(SO_NAMEx)
	@-rm -f $(INSTALL_LIB_DIR)/$(SO_NAMExx)
	@-rm -f $(INSTALL_LIB_DIR)/$(SO_NAMExxx)
	@-rm -f $(INSTALL_LIB_DIR_DBG)/$(SO_NAME_DBG)
	@-rm -f $(INSTALL_LIB_DIR_DBG)/$(SO_NAME_DBGx)
	@-rm -f $(INSTALL_LIB_DIR_DBG)/$(SO_NAME_DBGxx)
	@-rm -f $(INSTALL_LIB_DIR_DBG)/$(SO_NAME_DBGxxx)
	@-rm -f $(INSTALL_LIB_DIR_STLDBG)/$(SO_NAME_STLDBG)
	@-rm -f $(INSTALL_LIB_DIR_STLDBG)/$(SO_NAME_STLDBGx)
	@-rm -f $(INSTALL_LIB_DIR_STLDBG)/$(SO_NAME_STLDBGxx)
	@-rm -f $(INSTALL_LIB_DIR_STLDBG)/$(SO_NAME_STLDBGxxx)
	@-rm -f $(INSTALL_LIB_DIR)/${A_NAME_OUT}
	@-rm -f $(INSTALL_LIB_DIR_DBG)/${A_NAME_OUT_DBG}
	@-rm -f $(INSTALL_LIB_DIR_STLDBG)/${A_NAME_OUT_STLDBG}
	@-rmdir -p $(INSTALL_LIB_DIR) $(INSTALL_LIB_DIR_DBG) $(INSTALL_LIB_DIR_STLDBG) 2>/dev/null || exit 0


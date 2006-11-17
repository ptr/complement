# -*- makefile -*- Time-stamp: <06/11/17 00:29:39 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

define prog_clean
clean::
	@rm -f $${$(1)_PRG} $${$(1)_PRG_DBG} $${$(1)_PRG_STLDBG}

uninstall::
	@rm -f $$(INSTALL_BIN_DIR)/$$($(1)_PRG) $$(INSTALL_BIN_DIR_DBG)/$$($(1)_PRG_DBG) $$(INSTALL_BIN_DIR_STLDBG)/$$($(1)_PRG_STLDBG)
endef

clean::
ifdef PRGNAME
	@-rm -f ${PRG} ${PRG_DBG} ${PRG_STLDBG}
endif

$(foreach prg,$(PRGNAMES),$(eval $(call prog_clean,$(prg))))

distclean::
	@-rm -f $(DEPENDS_COLLECTION)
	@-rmdir -p ${OUTPUT_DIR} ${OUTPUT_DIR_DBG} ${OUTPUT_DIR_STLDBG} 2>/dev/null

uninstall::
ifdef PRGNAME
	@-rm -f $(INSTALL_BIN_DIR)/$(PRG) $(INSTALL_BIN_DIR_DBG)/$(PRG_DBG) $(INSTALL_BIN_DIR_STLDBG)/$(PRG_STLDBG)
endif
	@-rmdir -p $(INSTALL_BIN_DIR) $(INSTALL_BIN_DIR_DBG) $(INSTALL_BIN_DIR_STLDBG) 2>/dev/null

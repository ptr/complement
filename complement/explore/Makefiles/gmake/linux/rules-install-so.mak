# -*- makefile -*- Time-stamp: <06/11/03 11:47:01 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

ifndef WITHOUT_STLPORT
INSTALL_TAGS ?= install-release-shared install-dbg-shared install-stldbg-shared
else
INSTALL_TAGS ?= install-release-shared install-dbg-shared
endif

PHONY += install $(INSTALL_TAGS)

install:	$(INSTALL_TAGS)

# Workaround forGNU make 3.80; see comments in rules-so.mak
define do_install_so_links
$${INSTALL_LIB_DIR$(1)}/$${SO_NAME$(1)xxx}:	${SO_NAME_OUT$(1)xxx}
	$$(INSTALL_SO) $${SO_NAME_OUT$(1)xxx} $(INSTALL_LIB_DIR$(1))
	@$(call do_so_links_1,$$(INSTALL_LIB_DIR$(1)),$${SO_NAME$(1)xx},$${SO_NAME$(1)xxx})
	@$(call do_so_links_1,$$(INSTALL_LIB_DIR$(1)),$${SO_NAME$(1)x},$${SO_NAME$(1)xx})
	@$(call do_so_links_1,$$(INSTALL_LIB_DIR$(1)),$${SO_NAME$(1)},$${SO_NAME$(1)x})
endef

define do_install_so_links_wk
# expand to nothing, if WITHOUT_STLPORT
ifndef WITHOUT_STLPORT
$(call do_install_so_links,$(1))
endif
endef

$(eval $(call do_install_so_links,))
$(eval $(call do_install_so_links,_DBG))
# ifndef WITHOUT_STLPORT
$(eval $(call do_install_so_links_wk,_STLDBG))
# endif

install-release-shared:	release-shared $(INSTALL_LIB_DIR) $(INSTALL_LIB_DIR)/${SO_NAMExxx}
	${POST_INSTALL}

install-dbg-shared:	dbg-shared $(INSTALL_LIB_DIR_DBG) $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGxxx}
	${POST_INSTALL_DBG}

ifndef WITHOUT_STLPORT
install-stldbg-shared:	stldbg-shared $(INSTALL_LIB_DIR_STLDBG) $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGxxx}
	${POST_INSTALL_STLDBG}
endif

# -*- makefile -*- Time-stamp: <03/06/22 20:11:10 ptr>
# $Id$

include ${BASEDIR}/Makefiles/lib/aggregate.mak
include ${BASEDIR}/Makefiles/lib/macro.mak
include ${BASEDIR}/Makefiles/lib/rules-so.mak
include ${BASEDIR}/Makefiles/lib/rules-a.mak
include ${BASEDIR}/Makefiles/lib/rules-install-so.mak
include ${BASEDIR}/Makefiles/lib/rules-install-a.mak

prepare-pkg-shared:	release-shared
	$(INSTALL_SO) ${SO_NAME_SHORT} $(INSTALL_PRE_LIB_DIR)
	(cd $(INSTALL_PRE_LIB_DIR); \
	 rm -f ${SO_NAME_LINK}; \
	 ln -s ${SO_NAME_LONG} ${SO_NAME_LINK} )

clear-release-shared::
	@rm -f ${SO_NAME_SHORT}

clear-dbg-shared::
	@rm -f ${SO_NAME_OUT_DBG}
	@rm -f ${SO_NAME_OUT_DBGx}
	@rm -f ${SO_NAME_OUT_DBGxx}
	@rm -f ${SO_NAME_OUT_DBGxxx}

clear-stldbg-shared::
	@rm -f ${SO_NAME_SHORT_STLDBG}

clear-dbg-static::
	@rm -f ${A_NAME_SHORT_DBG}

clear-all:	clear
	-rm -f ${SO_INSTALL} ${SO_INSTALL_LN}

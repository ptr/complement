# -*- makefile -*- Time-stamp: <06/10/09 17:38:01 ptr>

INSTALL_TAGS ?= install-release-shared install-dbg-shared install-stldbg-shared

PHONY += install $(INSTALL_TAGS)

install:	$(INSTALL_TAGS)

$(INSTALL_LIB_DIR)/${SO_NAMExxx}:	${SO_NAME_OUTxxx}
	$(INSTALL_SO) ${SO_NAME_OUTxxx} $(INSTALL_LIB_DIR)
	@if [ -h $(INSTALL_LIB_DIR)/${SO_NAMExx} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR)/${SO_NAMExx}` != "${SO_NAMExxx}" ]; then \
	    rm $(INSTALL_LIB_DIR)/${SO_NAMExx}; \
	    ln -s ${SO_NAMExxx} $(INSTALL_LIB_DIR)/${SO_NAMExx}; \
	  fi \
	else \
	  ln -s ${SO_NAMExxx} $(INSTALL_LIB_DIR)/${SO_NAMExx}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR)/${SO_NAMEx} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR)/${SO_NAMEx}` != "${SO_NAMExx}" ]; then \
	    rm $(INSTALL_LIB_DIR)/${SO_NAMEx}; \
	    ln -s ${SO_NAMExx} $(INSTALL_LIB_DIR)/${SO_NAMEx}; \
	  fi \
	else \
	  ln -s ${SO_NAMExx} $(INSTALL_LIB_DIR)/${SO_NAMEx}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR)/${SO_NAME} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR)/${SO_NAME}` != "${SO_NAMEx}" ]; then \
	    rm $(INSTALL_LIB_DIR)/${SO_NAME}; \
	    ln -s ${SO_NAMEx} $(INSTALL_LIB_DIR)/${SO_NAME}; \
	  fi \
	else \
	  ln -s ${SO_NAMEx} $(INSTALL_LIB_DIR)/${SO_NAME}; \
	fi

install-release-shared:	release-shared $(INSTALL_LIB_DIR) $(INSTALL_LIB_DIR)/${SO_NAMExxx}
	${POST_INSTALL}

$(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGxxx}:	${SO_NAME_OUT_DBGxxx}
	$(INSTALL_SO) ${SO_NAME_OUT_DBGxxx} $(INSTALL_LIB_DIR_DBG)
	@if [ -h $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGxx} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGxx}` != "${SO_NAME_DBGxxx}" ]; then \
	    rm $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGxx}; \
	    ln -s ${SO_NAME_DBGxxx} $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGxx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGxxx} $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGxx}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGx} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGx}` != "${SO_NAME_DBGxx}" ]; then \
	    rm $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGx}; \
	    ln -s ${SO_NAME_DBGxx} $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGxx} $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGx}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBG} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBG}` != "${SO_NAME_DBGx}" ]; then \
	    rm $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBG}; \
	    ln -s ${SO_NAME_DBGx} $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBG}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGx} $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBG}; \
	fi


install-dbg-shared:	dbg-shared $(INSTALL_LIB_DIR_DBG) $(INSTALL_LIB_DIR_DBG)/${SO_NAME_DBGxxx}
	${POST_INSTALL_DBG}

$(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGxxx}:	${SO_NAME_OUT_STLDBGxxx}
	$(INSTALL_SO) ${SO_NAME_OUT_STLDBGxxx} $(INSTALL_LIB_DIR_STLDBG)
	@if [ -h $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGxx} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGxx}` != "${SO_NAME_STLDBGxxx}" ]; then \
	    rm $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGxx}; \
	    ln -s ${SO_NAME_STLDBGxxx} $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_DSTLBGxx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGxxx} $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_DSTLBGxx}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGx} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGx}` != "${SO_NAME_DBGxx}" ]; then \
	    rm $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	    ln -s ${SO_NAME_STLDBGxx} $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGxx} $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBG} ] ; then \
	  if [ `readlink $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBG}` != "${SO_NAME_STLDBGx}" ]; then \
	    rm $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	    ln -s ${SO_NAME_STLDBGx} $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGx} $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	fi

install-stldbg-shared:	stldbg-shared $(INSTALL_LIB_DIR_STLDBG) $(INSTALL_LIB_DIR_STLDBG)/${SO_NAME_STLDBGxxx}
	${POST_INSTALL_STLDBG}


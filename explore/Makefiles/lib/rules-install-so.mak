# -*- makefile -*- Time-stamp: <03/06/22 20:05:48 ptr>
# $Id$

install-release-shared:	release-shared
	@if [ -f ${SO_INSTALL} ] ; then \
	  rm ${SO_INSTALL}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR)/${SO_MAJOR} ] ; then \
	  rm $(INSTALL_LIB_DIR)/${SO_MAJOR}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR)/${SO_GENERAL} ] ; then \
	  rm $(INSTALL_LIB_DIR)/${SO_GENERAL}; \
	fi
	$(INSTALL_SO) ${SO_NAME_SHORT} $(INSTALL_LIB_DIR)
	(cd $(INSTALL_LIB_DIR) && \
	 { rm -f ${SO_MINOR}; ln -s ${SO_NAME_LONG} ${SO_MINOR}; \
	   rm -f ${SO_MAJOR}; ln -s ${SO_MINOR} ${SO_MAJOR}; \
	   rm -f ${SO_GENERAL}; ln -s ${SO_MAJOR} ${SO_GENERAL}; } )

install-dbg-shared:	dbg-shared
	@if [ -f ${SO_INSTALL_DBG} ] ; then \
	  rm ${SO_INSTALL_DBG}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR_DBG)/${SO_MAJOR_DBG} ] ; then \
	  rm $(INSTALL_LIB_DIR_DBG)/${SO_MAJOR_DBG}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR_DBG)/${SO_GENERAL_DBG} ] ; then \
	  rm $(INSTALL_LIB_DIR_DBG)/${SO_GENERAL_DBG}; \
	fi
	$(INSTALL_SO) ${SO_NAME_SHORT_DBG} $(INSTALL_LIB_DIR_DBG)
	(cd $(INSTALL_LIB_DIR_DBG) && \
	 { rm -f ${SO_MINOR_DBG}; \
	   ln -s ${SO_NAME_LONG_DBG} ${SO_MINOR_DBG}; \
	   rm -f ${SO_MAJOR_DBG}; \
	   ln -s ${SO_MINOR_DBG} ${SO_MAJOR_DBG}; \
	   rm -f ${SO_GENERAL_DBG}; \
	   ln -s ${SO_MAJOR_DBG} ${SO_GENERAL_DBG}; } )

install-stldbg-shared:	stldbg-shared
	@if [ -f ${SO_INSTALL_STLDBG} ] ; then \
	  rm ${SO_INSTALL_STLDBG}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR_STLDBG)/${SO_MAJOR_STLDBG} ] ; then \
	  rm $(INSTALL_LIB_DIR_STLDBG)/${SO_MAJOR_STLDBG}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR_STLDBG)/${SO_GENERAL_STLDBG} ] ; then \
	  rm $(INSTALL_LIB_DIR_STLDBG)/${SO_GENERAL_STLDBG}; \
	fi
	$(INSTALL_SO) ${SO_NAME_SHORT_STLDBG} $(INSTALL_LIB_DIR_STLDBG)
	(cd $(INSTALL_LIB_DIR_STLDBG) && \
	 { rm -f ${SO_MINOR_STLDBG}; \
	   ln -s ${SO_NAME_LONG_STLDBG} ${SO_MINOR_STLDBG}; \
	   rm -f ${SO_MAJOR_STLDBG}; \
	   ln -s ${SO_MINOR_STLDBG} ${SO_MAJOR_STLDBG}; \
	   rm -f ${SO_GENERAL_STLDBG}; \
	   ln -s ${SO_MAJOR_STLDBG} ${SO_GENERAL_STLDBG}; } )

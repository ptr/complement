# -*- makefile -*- Time-stamp: <03/07/07 08:33:49 ptr>
# $Id$

# Shared libraries tags

release-shared:	$(OUTPUT_DIR) ${SO_NAME_SHORT}

dbg-shared:	$(OUTPUT_DIR_DBG) ${SO_NAME_SHORT_DBG}

stldbg-shared:	$(OUTPUT_DIR_STLDBG) ${SO_NAME_SHORT_STLDBG}

${SO_NAME_SHORT}:	$(OBJ) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) $(OBJ) $(LDLIBS)
	(cd $(OUTPUT_DIR); \
	 rm -f ${SO_MINOR}; \
	 ln -s $(SO_NAME_LONG) ${SO_MINOR}; \
	 rm -f ${SO_MAJOR}; \
	 ln -s ${SO_MINOR} ${SO_MAJOR}; \
	 rm -f ${SO_GENERAL}; \
         ln -s ${SO_MAJOR} ${SO_GENERAL})

${SO_NAME_SHORT_DBG}:	$(OBJ_DBG) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) $(OBJ_DBG) $(LDLIBS)
	(cd $(OUTPUT_DIR_DBG); \
	 rm -f ${SO_MINOR_DBG}; \
	 ln -s $(SO_NAME_LONG_DBG) ${SO_MINOR_DBG}; \
	 rm -f ${SO_MAJOR_DBG}; \
	 ln -s ${SO_MINOR_DBG} ${SO_MAJOR_DBG}; \
	 rm -f ${SO_GENERAL_DBG}; \
	 ln -s ${SO_MAJOR_DBG} ${SO_GENERAL_DBG})

${SO_NAME_SHORT_STLDBG}:	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) $(OBJ_STLDBG) $(LDLIBS)
	(cd $(OUTPUT_DIR_STLDBG); \
	 rm -f ${SO_MINOR_STLDBG}; \
	 ln -s $(SO_NAME_LONG_STLDBG) ${SO_MINOR_STLDBG}; \
	 rm -f ${SO_MAJOR_STLDBG}; \
	 ln -s ${SO_MINOR_STLDBG} ${SO_MAJOR_STLDBG}; \
	 rm -f ${SO_GENERAL_STLDBG}; \
	 ln -s ${SO_MAJOR_STLDBG} ${SO_GENERAL_STLDBG})


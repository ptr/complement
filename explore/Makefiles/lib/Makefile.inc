# -*- makefile -*- Time-stamp: <02/03/11 15:28:35 ptr>
# $Id$

include $(BASEDIR)/Makefiles/Makefile.inc
-include $(BASEDIR)/Makefiles/Makefile.dep.${COMPILER_NAME}
include ${BASEDIR}/Makefiles/Makefile.inc.${COMPILER_NAME}
include $(BASEDIR)/Makefiles/Makefile.inc.ver
LDFLAGS = 
include ${BASEDIR}/Makefiles/lib/Makefile.inc.${COMPILER_NAME}

# Shared libraries:
# SO_NAME_SHORT = lib${LIBNAME}${DBG_SUFFIX}.so
SO_NAME_SHORT = $(OUTPUT_DIR)/lib${LIBNAME}_${COMPILER_NAME}.$(SO)${MAJOR}${MINOR}${PATCH}
SO_NAME_SHORT_DBG = $(OUTPUT_DIR_DBG)/lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(SO)${MAJOR}${MINOR}${PATCH}
SO_NAME_SHORT_STLDBG = $(OUTPUT_DIR_STLDBG)/lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(SO)${MAJOR}${MINOR}${PATCH}

SO_NAME_LONG = lib${LIBNAME}_${COMPILER_NAME}.$(SO)${MAJOR}${MINOR}${PATCH}
# SO_NAME_LONG  = ${SO_NAME_SHORT}${MAJOR}${MINOR}${CCID}
  # The name in the so library for linker
SO_NAME_LINK = lib${LIBNAME}_${COMPILER_NAME}.$(SO)${MAJOR}${MINOR}

SO_NAME_LONG_DBG = lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(SO)${MAJOR}${MINOR}${PATCH}
  # The name in the so library for linker
SO_NAME_LINK_DBG = lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(SO)${MAJOR}${MINOR}

SO_NAME_LONG_STLDBG = lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(SO)${MAJOR}${MINOR}${PATCH}
  # The name in the so library for linker
SO_NAME_LINK_STLDBG = lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(SO)${MAJOR}${MINOR}

SO_INSTALL = $(INSTALL_LIB_DIR)/${SO_NAME_LONG}
SO_MINOR = lib${LIBNAME}_${COMPILER_NAME}.$(SO)${MAJOR}${MINOR}
SO_MAJOR = lib${LIBNAME}_${COMPILER_NAME}.$(SO)${MAJOR}
SO_GENERAL = lib${LIBNAME}_${COMPILER_NAME}.$(SO)

SO_INSTALL_DBG = $(INSTALL_LIB_DIR)/${SO_NAME_LONG_DBG}
SO_MINOR_DBG = lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(SO)${MAJOR}${MINOR}
SO_MAJOR_DBG = lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(SO)${MAJOR}
SO_GENERAL_DBG = lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(SO)

SO_INSTALL_STLDBG = $(INSTALL_LIB_DIR)/${SO_NAME_LONG_STLDBG}
SO_MINOR_STLDBG = lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(SO)${MAJOR}${MINOR}
SO_MAJOR_STLDBG = lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(SO)${MAJOR}
SO_GENERAL_STLDBG = lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(SO)

# Static libraries:
A_NAME_SHORT = $(OUTPUT_DIR_A)/lib${LIBNAME}_${COMPILER_NAME}.$(ARCH)
A_NAME_SHORT_DBG = $(OUTPUT_DIR_A_DBG)/lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(ARCH)
A_NAME_SHORT_STLDBG = $(OUTPUT_DIR_A_STLDBG)/lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(ARCH)

A_NAME_LONG = lib${LIBNAME}_${COMPILER_NAME}.$(ARCH)
A_NAME_LONG_DBG = lib${LIBNAME}_${COMPILER_NAME}${DBG_SUFFIX}.$(ARCH)
A_NAME_LONG_STLDBG = lib${LIBNAME}_${COMPILER_NAME}_stl${DBG_SUFFIX}.$(ARCH)

A_INSTALL = $(INSTALL_LIB_DIR)/${A_NAME_LONG}
A_INSTALL_DBG = $(INSTALL_LIB_DIR)/${A_NAME_LONG_DBG}
A_INSTALL_STLDBG = $(INSTALL_LIB_DIR)/${A_NAME_LONG_STLDBG}


# Shared libraries tags

release-shared:	$(OUTPUT_DIR) ${SO_NAME_SHORT}

dbg-shared:	$(OUTPUT_DIR_DBG) ${SO_NAME_SHORT_DBG}

stldbg-shared:	$(OUTPUT_DIR_STLDBG) ${SO_NAME_SHORT_STLDBG}

release-shared-dep:	$(OUTPUT_DIR) $(DEP)

dbg-shared-dep:	$(OUTPUT_DIR_DBG) $(DEP_DBG)

stldbg-shared-dep:	$(OUTPUT_DIR_STLDBG) $(DEP_STLDBG)

# Static libraries tags

release-static:	$(OUTPUT_DIR_A) ${A_NAME_SHORT}

dbg-static:	$(OUTPUT_DIR_A_DBG) ${A_NAME_SHORT_DBG}

stldbg-static:	$(OUTPUT_DIR_A_STLDBG) ${A_NAME_SHORT_STLDBG}

release-static-dep:	$(OUTPUT_DIR_A) $(DEP_A)

dbg-static-dep:	$(OUTPUT_DIR_A_DBG) $(DEP_A_DBG)

stldbg-static-dep:	$(OUTPUT_DIR_A_STLDBG) $(DEP_A_STLDBG)


${SO_NAME_SHORT}:	$(OBJ) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ) $(LDLIBS)
	(cd $(OUTPUT_DIR); \
	 rm -f ${SO_MINOR}; \
	 ln -s $(SO_NAME_LONG) ${SO_MINOR}; \
	 rm -f ${SO_MAJOR}; \
	 ln -s ${SO_MINOR} ${SO_MAJOR}; \
	 rm -f ${SO_GENERAL}; \
         ln -s ${SO_MAJOR} ${SO_GENERAL})

${SO_NAME_SHORT_DBG}:	$(OBJ_DBG) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_DBG) $(LDLIBS)
	(cd $(OUTPUT_DIR_DBG); \
	 rm -f ${SO_MINOR_DBG}; \
	 ln -s $(SO_NAME_LONG_DBG) ${SO_MINOR_DBG}; \
	 rm -f ${SO_MAJOR_DBG}; \
	 ln -s ${SO_MINOR_DBG} ${SO_MAJOR_DBG}; \
	 rm -f ${SO_GENERAL_DBG}; \
	 ln -s ${SO_MAJOR_DBG} ${SO_GENERAL_DBG})

${SO_NAME_SHORT_STLDBG}:	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK.cc) $(OUTPUT_OPTION) $(OBJ_STLDBG) $(LDLIBS)
	(cd $(OUTPUT_DIR_STLDBG); \
	 rm -f ${SO_MINOR_STLDBG}; \
	 ln -s $(SO_NAME_LONG_STLDBG) ${SO_MINOR_STLDBG}; \
	 rm -f ${SO_MAJOR_STLDBG}; \
	 ln -s ${SO_MINOR_STLDBG} ${SO_MAJOR_STLDBG}; \
	 rm -f ${SO_GENERAL_STLDBG}; \
	 ln -s ${SO_MAJOR_STLDBG} ${SO_GENERAL_STLDBG})

${A_NAME_SHORT}:	$(OBJ_A)
	$(AR) -r ${A_NAME_SHORT} $(OBJ_A)

${A_NAME_SHORT_DBG}:	$(OBJ_A_DBG)
	$(AR) -r ${A_NAME_SHORT_DBG} $(OBJ_A_DBG)

${A_NAME_SHORT_STLDBG}:	$(OBJ_A_STLDBG)
	$(AR) -r ${A_NAME_SHORT_STLDBG} $(OBJ_A_STLDBG)

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

prepare-pkg-shared:	release-shared
	$(INSTALL_SO) ${SO_NAME_SHORT} $(INSTALL_PRE_LIB_DIR)
	(cd $(INSTALL_PRE_LIB_DIR); \
	 rm -f ${SO_NAME_LINK}; \
	 ln -s ${SO_NAME_LONG} ${SO_NAME_LINK} )

install-release-static:	release-static
	@if [ -f ${A_INSTALL} ] ; then \
	  rm ${A_INSTALL}; \
	fi
	$(INSTALL_A) ${A_NAME_SHORT} $(INSTALL_LIB_DIR)

install-dbg-shared:	dbg-shared
	@if [ -f ${SO_INSTALL_DBG} ] ; then \
	  rm ${SO_INSTALL_DBG}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR)/${SO_MAJOR_DBG} ] ; then \
	  rm $(INSTALL_LIB_DIR)/${SO_MAJOR_DBG}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR)/${SO_GENERAL_DBG} ] ; then \
	  rm $(INSTALL_LIB_DIR)/${SO_GENERAL_DBG}; \
	fi
	$(INSTALL_SO) ${SO_NAME_SHORT_DBG} $(INSTALL_LIB_DIR)
	(cd $(INSTALL_LIB_DIR) && \
	 { rm -f ${SO_MINOR_DBG}; \
	   ln -s ${SO_NAME_LONG_DBG} ${SO_MINOR_DBG}; \
	   rm -f ${SO_MAJOR_DBG}; \
	   ln -s ${SO_MINOR_DBG} ${SO_MAJOR_DBG}; \
	   rm -f ${SO_GENERAL_DBG}; \
	   ln -s ${SO_MAJOR_DBG} ${SO_GENERAL_DBG}; } )

install-dbg-static:	dbg-static
	@if [ -f ${A_INSTALL_DBG} ] ; then \
	  rm ${A_INSTALL_DBG}; \
	fi
	$(INSTALL_A) ${A_NAME_SHORT_DBG} $(INSTALL_LIB_DIR)

install-stldbg-shared:	stldbg-shared
	@if [ -f ${SO_INSTALL_STLDBG} ] ; then \
	  rm ${SO_INSTALL_STLDBG}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR)/${SO_MAJOR_STLDBG} ] ; then \
	  rm $(INSTALL_LIB_DIR)/${SO_MAJOR_STLDBG}; \
	fi
	@if [ -h $(INSTALL_LIB_DIR)/${SO_GENERAL_STLDBG} ] ; then \
	  rm $(INSTALL_LIB_DIR)/${SO_GENERAL_STLDBG}; \
	fi
	$(INSTALL_SO) ${SO_NAME_SHORT_STLDBG} $(INSTALL_LIB_DIR)
	(cd $(INSTALL_LIB_DIR) && \
	 { rm -f ${SO_MINOR_STLDBG}; \
	   ln -s ${SO_NAME_LONG_STLDBG} ${SO_MINOR_STLDBG}; \
	   rm -f ${SO_MAJOR_STLDBG}; \
	   ln -s ${SO_MINOR_STLDBG} ${SO_MAJOR_STLDBG}; \
	   rm -f ${SO_GENERAL_STLDBG}; \
	   ln -s ${SO_MAJOR_STLDBG} ${SO_GENERAL_STLDBG}; } )

install-stldbg-static:	stldbg-static
	@if [ -f ${A_INSTALL_STLDBG} ] ; then \
	  rm ${A_INSTALL_STLDBG}; \
	fi
	$(INSTALL_A) ${A_NAME_SHORT_STLDBG} $(INSTALL_LIB_DIR)

clear-release-shared::
	@rm -f ${SO_NAME_SHORT}

clear-release-static::
	@rm -f ${A_NAME_SHORT}

clear-dbg-shared::
	@rm -f ${SO_NAME_SHORT_DBG}

clear-stldbg-shared::
	@rm -f ${SO_NAME_SHORT_STLDBG}

clear-dbg-static::
	@rm -f ${A_NAME_SHORT_DBG}

clear-stldbg-static::
	@rm -f ${A_NAME_SHORT_STLDBG}

clear-static:	clear-dbg-static clear-stldbg-static \
		clear-release-static

clear-all:	clear-static clear
	-rm -f ${SO_INSTALL} ${SO_INSTALL_LN}

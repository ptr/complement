# -*- makefile -*- Time-stamp: <06/10/09 17:36:05 ptr>

# Shared libraries tags

PHONY += release-shared dbg-shared stldbg-shared

release-shared:	$(OUTPUT_DIR) ${SO_NAME_OUTxxx}

dbg-shared:	$(OUTPUT_DIR_DBG) ${SO_NAME_OUT_DBGxxx}

stldbg-shared:	$(OUTPUT_DIR_STLDBG) ${SO_NAME_OUT_STLDBGxxx}

ifeq ("${_C_SOURCES_ONLY}","")

${SO_NAME_OUTxxx}:	$(OBJ) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) ${START_OBJ} $(OBJ) $(LDLIBS) ${STDLIBS} ${END_OBJ}
	@if [ -h $(OUTPUT_DIR)/${SO_NAMExx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR)/${SO_NAMExx}` != "${SO_NAMExxx}" ]; then \
	    rm $(OUTPUT_DIR)/${SO_NAMExx}; \
	    ln -s ${SO_NAMExxx} $(OUTPUT_DIR)/${SO_NAMExx}; \
	  fi \
	else \
	  ln -s ${SO_NAMExxx} $(OUTPUT_DIR)/${SO_NAMExx}; \
	fi
	@if [ -h $(OUTPUT_DIR)/${SO_NAMEx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR)/${SO_NAMEx}` != "${SO_NAMExx}" ]; then \
	    rm $(OUTPUT_DIR)/${SO_NAMEx}; \
	    ln -s ${SO_NAMExx} $(OUTPUT_DIR)/${SO_NAMEx}; \
	  fi \
	else \
	  ln -s ${SO_NAMExx} $(OUTPUT_DIR)/${SO_NAMEx}; \
	fi
	@if [ -h $(OUTPUT_DIR)/${SO_NAME} ] ; then \
	  if [ `readlink $(OUTPUT_DIR)/${SO_NAME}` != "${SO_NAMEx}" ]; then \
	    rm $(OUTPUT_DIR)/${SO_NAME}; \
	    ln -s ${SO_NAMEx} $(OUTPUT_DIR)/${SO_NAME}; \
	  fi \
	else \
	  ln -s ${SO_NAMEx} $(OUTPUT_DIR)/${SO_NAME}; \
	fi

${SO_NAME_OUT_DBGxxx}:	$(OBJ_DBG) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) ${START_OBJ} $(OBJ_DBG) $(LDLIBS) ${STDLIBS} ${END_OBJ}
	@if [ -h $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}` != "${SO_NAME_DBGxxx}" ]; then \
	    rm $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}; \
	    ln -s ${SO_NAME_DBGxxx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGxxx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}; \
	fi
	@if [ -h $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}` != "${SO_NAME_DBGxx}" ]; then \
	    rm $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}; \
	    ln -s ${SO_NAME_DBGxx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGxx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}; \
	fi
	@if [ -h $(OUTPUT_DIR_DBG)/${SO_NAME_DBG} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}` != "${SO_NAME_DBGx}" ]; then \
	    rm $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}; \
	    ln -s ${SO_NAME_DBGx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}; \
	fi

${SO_NAME_OUT_STLDBGxxx}:	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK.cc) $(LINK_OUTPUT_OPTION) ${START_OBJ} $(OBJ_STLDBG) $(LDLIBS) ${STDLIBS} ${END_OBJ}
	@if [ -h $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}` != "${SO_NAME_STLDBGxxx}" ]; then \
	    rm $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}; \
	    ln -s ${SO_NAME_STLDBGxxx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGxxx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}; \
	fi
	@if [ -h $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}` != "${SO_NAME_STLDBGxx}" ]; then \
	    rm $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	    ln -s ${SO_NAME_STLDBGxx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGxx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	fi
	@if [ -h $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}` != "${SO_NAME_STLDBGx}" ]; then \
	    rm $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	    ln -s ${SO_NAME_STLDBGx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	fi

else

${SO_NAME_OUTxxx}:	$(OBJ) $(LIBSDEP)
	$(LINK.c) $(LINK_OUTPUT_OPTION) $(OBJ) $(LDLIBS)
	@if [ -h $(OUTPUT_DIR)/${SO_NAMExx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR)/${SO_NAMExx}` != "${SO_NAMExxx}" ]; then \
	    rm $(OUTPUT_DIR)/${SO_NAMExx}; \
	    ln -s ${SO_NAMExxx} $(OUTPUT_DIR)/${SO_NAMExx}; \
	  fi \
	else \
	  ln -s ${SO_NAMExxx} $(OUTPUT_DIR)/${SO_NAMExx}; \
	fi
	@if [ -h $(OUTPUT_DIR)/${SO_NAMEx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR)/${SO_NAMEx}` != "${SO_NAMExx}" ]; then \
	    rm $(OUTPUT_DIR)/${SO_NAMEx}; \
	    ln -s ${SO_NAMExx} $(OUTPUT_DIR)/${SO_NAMEx}; \
	  fi \
	else \
	  ln -s ${SO_NAMExx} $(OUTPUT_DIR)/${SO_NAMEx}; \
	fi
	@if [ -h $(OUTPUT_DIR)/${SO_NAME} ] ; then \
	  if [ `readlink $(OUTPUT_DIR)/${SO_NAME}` != "${SO_NAMEx}" ]; then \
	    rm $(OUTPUT_DIR)/${SO_NAME}; \
	    ln -s ${SO_NAMEx} $(OUTPUT_DIR)/${SO_NAME}; \
	  fi \
	else \
	  ln -s ${SO_NAMEx} $(OUTPUT_DIR)/${SO_NAME}; \
	fi

${SO_NAME_OUT_DBGxxx}:	$(OBJ_DBG) $(LIBSDEP)
	$(LINK.c) $(LINK_OUTPUT_OPTION) $(OBJ_DBG) $(LDLIBS)
	@if [ -h $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}` != "${SO_NAME_DBGxxx}" ]; then \
	    rm $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}; \
	    ln -s ${SO_NAME_DBGxxx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGxxx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBGxx}; \
	fi
	@if [ -h $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}` != "${SO_NAME_DBGxx}" ]; then \
	    rm $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}; \
	    ln -s ${SO_NAME_DBGxx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGxx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBGx}; \
	fi
	@if [ -h $(OUTPUT_DIR_DBG)/${SO_NAME_DBG} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}` != "${SO_NAME_DBGx}" ]; then \
	    rm $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}; \
	    ln -s ${SO_NAME_DBGx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}; \
	  fi \
	else \
	  ln -s ${SO_NAME_DBGx} $(OUTPUT_DIR_DBG)/${SO_NAME_DBG}; \
	fi

${SO_NAME_OUT_STLDBGxxx}:	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK.c) $(LINK_OUTPUT_OPTION) $(OBJ_STLDBG) $(LDLIBS)
	@if [ -h $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}` != "${SO_NAME_STLDBGxxx}" ]; then \
	    rm $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}; \
	    ln -s ${SO_NAME_STLDBGxxx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGxxx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGxx}; \
	fi
	@if [ -h $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}` != "${SO_NAME_STLDBGxx}" ]; then \
	    rm $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	    ln -s ${SO_NAME_STLDBGxx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGxx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBGx}; \
	fi
	@if [ -h $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG} ] ; then \
	  if [ `readlink $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}` != "${SO_NAME_STLDBGx}" ]; then \
	    rm $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	    ln -s ${SO_NAME_STLDBGx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	  fi \
	else \
	  ln -s ${SO_NAME_STLDBGx} $(OUTPUT_DIR_STLDBG)/${SO_NAME_STLDBG}; \
	fi

endif

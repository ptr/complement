# -*- makefile -*- Time-stamp: <03/09/28 16:54:43 ptr>
# $Id$

# Shared libraries tags

release-shared:	$(OUTPUT_DIR) $(SO_NAME_OUT)

dbg-shared:	$(OUTPUT_DIR_DBG) $(SO_NAME_OUT_DBG)

stldbg-shared:	$(OUTPUT_DIR_STLDBG) $(SO_NAME_OUT_STLDBG)

# .o{$(OUTPUT_DIR)}.o:

$(SO_NAME_OUT):	$(OBJ) $(LIBSDEP)
	$(LINK_cc_REL) $(LINK_OUTPUT_OPTION) $(OBJ) $(LDLIBS)

$(SO_NAME_OUT_DBG):	$(OBJ_DBG) $(LIBSDEP)
	$(LINK_cc_DBG) $(LINK_OUTPUT_OPTION) $(OBJ_DBG) $(LDLIBS)

$(SO_NAME_OUT_STLDBG):	$(OBJ_STLDBG) $(LIBSDEP)
	$(LINK_cc_STLDBG) $(LINK_OUTPUT_OPTION) $(OBJ_STLDBG) $(LDLIBS)


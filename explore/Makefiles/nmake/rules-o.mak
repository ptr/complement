# -*- Makefile -*- Time-stamp: <03/09/28 16:52:08 ptr>
# $Id$

# Rules for release output:

.cpp{$(OUTPUT_DIR)}.o:
	$(COMPILE_cc_REL) $(OUTPUT_OPTION) $<

.cc{$(OUTPUT_DIR)}.o:
	$(COMPILE_cc_REL) $(OUTPUT_OPTION) $<

.c{$(OUTPUT_DIR)}.o:
	$(COMPILE_c_REL) $(OUTPUT_OPTION) $<

# Rules for debug output:

.cpp{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_cc_DBG) $(OUTPUT_OPTION_DBG) $<

.cc{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_cc_DBG) $(OUTPUT_OPTION_DBG) $<

.c{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_c_DBG) $(OUTPUT_OPTION_DBG) $<

# Rules for STLport debug output:

.cpp{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc_STLDBG) $(OUTPUT_OPTION_STLDBG) $<

.cc{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc_STLDBG) $(OUTPUT_OPTION_STLDBG) $<

.c{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_c_STL_DBG) $(OUTPUT_OPTION_STLDBG) $<


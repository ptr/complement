# -*- Makefile -*- Time-stamp: <03/09/15 14:22:01 ptr>
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
	$(COMPILE_cc) $(OUTPUT_OPTION) $<

.cc{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_cc) $(OUTPUT_OPTION) $<

.c{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_c) $(OUTPUT_OPTION) $<

# Rules for STLport debug output:

.cpp{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc) $(OUTPUT_OPTION) $<

.cc{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc) $(OUTPUT_OPTION) $<

.c{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_c) $(OUTPUT_OPTION) $<


# -*- Makefile -*- Time-stamp: <03/10/13 15:27:30 ptr>
# $Id$

# Rules for release output:

.cpp{$(OUTPUT_DIR)}.o:
	$(COMPILE_cc_REL) $(OUTPUT_OPTION) $<

{..}.cpp{$(OUTPUT_DIR)}.o:
	$(COMPILE_cc_REL) $(OUTPUT_OPTION) $<

.cc{$(OUTPUT_DIR)}.o:
	$(COMPILE_cc_REL) $(OUTPUT_OPTION) $<

{..}.cc{$(OUTPUT_DIR)}.o:
	$(COMPILE_cc_REL) $(OUTPUT_OPTION) $<

.c{$(OUTPUT_DIR)}.o:
	$(COMPILE_c_REL) $(OUTPUT_OPTION) $<

{..}.c{$(OUTPUT_DIR)}.o:
	$(COMPILE_c_REL) $(OUTPUT_OPTION) $<

.rc{$(OUTPUT_DIR)}.res:
	$(COMPILE_rc_REL) $(RC_OUTPUT_OPTION) $<

{..}.rc{$(OUTPUT_DIR)}.res:
	$(COMPILE_rc_REL) $(RC_OUTPUT_OPTION) $<

# Rules for debug output:

.cpp{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_cc_DBG) $(OUTPUT_OPTION_DBG) $<

{..}.cpp{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_cc_DBG) $(OUTPUT_OPTION_DBG) $<

.cc{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_cc_DBG) $(OUTPUT_OPTION_DBG) $<

{..}.cc{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_cc_DBG) $(OUTPUT_OPTION_DBG) $<

.c{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_c_DBG) $(OUTPUT_OPTION_DBG) $<

{..}.c{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_c_DBG) $(OUTPUT_OPTION_DBG) $<

.rc{$(OUTPUT_DIR_DBG)}.res:
	$(COMPILE_rc_DBG) $(RC_OUTPUT_OPTION_DBG) $<

{..}.rc{$(OUTPUT_DIR_DBG)}.res:
	$(COMPILE_rc_DBG) $(RC_OUTPUT_OPTION_DBG) $<

# Rules for STLport debug output:

.cpp{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc_STLDBG) $(OUTPUT_OPTION_STLDBG) $<

{..}.cpp{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc_STLDBG) $(OUTPUT_OPTION_STLDBG) $<

.cc{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc_STLDBG) $(OUTPUT_OPTION_STLDBG) $<

{..}.cc{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc_STLDBG) $(OUTPUT_OPTION_STLDBG) $<

.c{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_c_DBG) $(OUTPUT_OPTION_STLDBG) $<

{..}.c{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_c_DBG) $(OUTPUT_OPTION_STLDBG) $<

.rc{$(OUTPUT_DIR_STLDBG)}.res:
	$(COMPILE_rc_STLDBG) $(RC_OUTPUT_OPTION_STLDBG) $<

{..}.rc{$(OUTPUT_DIR_STLDBG)}.res:
	$(COMPILE_rc_STLDBG) $(RC_OUTPUT_OPTION_STLDBG) $<

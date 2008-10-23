# -*- makefile -*- Time-stamp: <03/10/17 17:16:23 ptr>
# $Id$

{$(BOOST_TST_SRC)}.cpp{$(OUTPUT_DIR)}.o:
	$(COMPILE_cc_REL) $(OUTPUT_OPTION) $<

!if "$(OUTPUT_DIR_A)" != "$(OUTPUT_DIR)"
# Rules for debug output (static):
{$(BOOST_TST_SRC)}.cpp{$(OUTPUT_DIR_A)}.o:
	$(COMPILE_cc_STATIC_REL) $(OUTPUT_OPTION) $<

!endif

{$(BOOST_TST_SRC)}.cpp{$(OUTPUT_DIR_DBG)}.o:
	$(COMPILE_cc_DBG) $(OUTPUT_OPTION_DBG) $<

!if "$(OUTPUT_DIR_A_DBG)" != "$(OUTPUT_DIR_DBG)"
# Rules for debug output (static):
{$(BOOST_TST_SRC)}.cpp{$(OUTPUT_DIR_A_DBG)}.o:
	$(COMPILE_cc_STATIC_DBG) $(OUTPUT_OPTION_DBG) $<

!endif

{$(BOOST_TST_SRC)}.cpp{$(OUTPUT_DIR_STLDBG)}.o:
	$(COMPILE_cc_STLDBG) $(OUTPUT_OPTION_STLDBG) $<

!if "$(OUTPUT_DIR_A_STLDBG)" != "$(OUTPUT_DIR_STLDBG)"
# Rules for STLport debug output (static):
{$(BOOST_TST_SRC)}.cpp{$(OUTPUT_DIR_A_STLDBG)}.o:
	$(COMPILE_cc_STATIC_STLDBG) $(OUTPUT_OPTION_STLDBG) $<

!endif

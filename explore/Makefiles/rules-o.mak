# -*- Makefile -*- Time-stamp: <03/07/04 12:59:26 ptr>
# $Id$

# Rules for release output:

$(OUTPUT_DIR)/%.o:	$(WORD1)%.cpp
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR)/%.o:	$(WORD1)%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR)/%.o:	$(WORD1)%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

# Rules for debug output:

$(OUTPUT_DIR_DBG)/%.o:	$(WORD1)%.cpp
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_DBG)/%.o:	$(WORD1)%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_DBG)/%.o:	$(WORD1)%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

# Rules for STLport debug output:

$(OUTPUT_DIR_STLDBG)/%.o:	$(WORD1)%.cpp
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_STLDBG)/%.o:	$(WORD1)%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR_STLDBG)/%.o:	$(WORD1)%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<


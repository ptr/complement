# Time-stamp: <03/09/15 13:28:34 ptr>
# $Id$

# dependency output parser
#!include ${RULESBASE}/dparser-$(COMPILER_NAME).mak

# if sources disposed in several dirs, calculate
# appropriate rules; here is recursive call!

#DIRS_UNIQUE_SRC := $(dir $(SRC_CPP) $(SRC_CC) $(SRC_C) )
#DIRS_UNIQUE_SRC := $(sort $(DIRS_UNIQUE_SRC) )
#include ${RULESBASE}/dirsrc.mak
!include $(RULESBASE)/$(USE_MAKE)/rules-o.mak

#ALLBASE    := $(basename $(notdir $(SRC_CC) $(SRC_CPP) $(SRC_C)))
ALLBASE    = $(SRC_CC) $(SRC_CPP) $(SRC_C)
#ALLOBJS    := $(addsuffix .o,$(ALLBASE))
ALLOBJS    = $(SRC_CC:.cc=.o) $(SRC_CPP:.cpp=.o) $(SRC_C:.c=.o)
#!if [echo $(ALLOBJS)]
#!endif
ALLDEPS    = $(SRC_CC:.cc=.d) $(SRC_CPP:.cpp=.d) $(SRC_C:.c=.d)
#OBJ        := $(addprefix $(OUTPUT_DIR)/,$(ALLOBJS))
#OBJ        = $(ALLOBJS)

#XOBJ        = {$(ALLOBJS)}
#!if [echo $(XOBJ)]
#!endif
#XOBJ        = $(ALLOBJS: =obj/vc6/shared/)

# Following trick intended to add prefix
# set marker (spaces are significant here!):
OBJ=$(ALLOBJS:.o =.o@)
# remove trailing marker (with white space):
OBJ=$(OBJ:.o@ =.o)
# replace marker by prefix:
#OBJ=$(OBJ:.o@=.o %OUTPUT_DIR%/)
# sorry, but I still not know how substitute macros in braces ();
OBJ=$(OBJ:.o@=.o obj/vc6/shared/)
# add prefix to first element:
OBJ=$(OUTPUT_DIR)/$(OBJ)
#!if [echo -$(OBJ)-]
#!endif

#OBJ_A      := $(OBJ)
#OBJ_DBG    := $(addprefix $(OUTPUT_DIR_DBG)/,$(ALLOBJS))
#OBJ_A_DBG  := $(OBJ_DBG)
#OBJ_STLDBG := $(addprefix $(OUTPUT_DIR_STLDBG)/,$(ALLOBJS))
#OBJ_A_STLDBG := $(OBJ_STLDBG)
#DEP        := $(addprefix $(OUTPUT_DIR)/,$(ALLDEPS))
#DEP_DBG    := $(addprefix $(OUTPUT_DIR_DBG)/,$(ALLDEPS))
#DEP_STLDBG := $(addprefix $(OUTPUT_DIR_STLDBG)/,$(ALLDEPS))


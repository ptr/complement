# Time-stamp: <03/09/28 19:19:30 ptr>
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

# follow tricks to avoid leading space if one of the macro undefined:
# SRC_CC, SRC_CPP or SRC_C
!ifdef SRC_CC
ALLOBJS    = $(SRC_CC:.cc=.o)
!endif
!ifdef SRC_CPP
!ifdef ALLOBJS
ALLOBJS = $(ALLOBJS) $(SRC_CPP:.cpp=.o)
!else
ALLOBJS = $(SRC_CPP:.cpp=.o)
!endif
!endif
!ifdef SRC_C
!ifdef ALLOBJS
ALLOBJS = $(ALLOBJS) $(SRC_C:.c=.o)
!else
ALLOBJS = $(SRC_C:.c=.o)
!endif
!endif

ALLDEPS    = $(SRC_CC:.cc=.d) $(SRC_CPP:.cpp=.d) $(SRC_C:.c=.d)

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
# !if [echo -$(OBJ)-]
# !endif

# The same trick for OBJ_DBG:
OBJ_DBG=$(ALLOBJS:.o =.o@)
OBJ_DBG=$(OBJ_DBG:.o@ =.o)
#OBJ=$(OBJ:.o@=.o %OUTPUT_DIR%/)
# sorry, but I still not know how substitute macros in braces ();
OBJ_DBG=$(OBJ_DBG:.o@=.o obj/vc6/shared-g/)
# add prefix to first element:
OBJ_DBG=$(OUTPUT_DIR_DBG)/$(OBJ_DBG)

# And for OBJ_STLDBG too:
OBJ_STLDBG=$(ALLOBJS:.o =.o@)
OBJ_STLDBG=$(OBJ_STLDBG:.o@ =.o)
#OBJ=$(OBJ:.o@=.o %OUTPUT_DIR%/)
# sorry, but I still not know how substitute macros in braces ();
OBJ_STLDBG=$(OBJ_STLDBG:.o@=.o obj/vc6/shared-stlg/)
# add prefix to first element:
OBJ_STLDBG=$(OUTPUT_DIR_STLDBG)/$(OBJ_STLDBG)

#OBJ_A      := $(OBJ)
#OBJ_DBG    := $(addprefix $(OUTPUT_DIR_DBG)/,$(ALLOBJS))
#OBJ_A_DBG  := $(OBJ_DBG)
#OBJ_STLDBG := $(addprefix $(OUTPUT_DIR_STLDBG)/,$(ALLOBJS))
#OBJ_A_STLDBG := $(OBJ_STLDBG)
#DEP        := $(addprefix $(OUTPUT_DIR)/,$(ALLDEPS))
#DEP_DBG    := $(addprefix $(OUTPUT_DIR_DBG)/,$(ALLDEPS))
#DEP_STLDBG := $(addprefix $(OUTPUT_DIR_STLDBG)/,$(ALLDEPS))


# Time-stamp: <99/03/19 16:58:12 ptr>
# %Z% $Date$ $Revision$ $RCSfile$ %Q%

PWD :sh = pwd
BASEDIR = $(PWD:%/lib/EDS=%)

include ../../Makefile.inc

INCLUDES += -I../../include/ios \
	    -I../../include/mt \
	    -I../../include/sockios \
	    -I../../include/EDS

EDS_SRC = _EventHandler.cc NetTransport.cc EvManager.cc EvPack.cc
EDS_OBJ = $(EDS_SRC:%.cc=$(OUTPUT_DIR)/%.o)

libEDS_dbg.a := OPT +=
libEDS_dbg.a := DEFS +=
libEDS_dbg.a := LDFLAGS += -xar

libEDS_dbg.a:	$(EDS_OBJ)
	$(LINK.cc) $(OUTPUT_OPTION) $(EDS_OBJ) $(LDLIBS)
	?-rm -f ../libEDS_dbg.a
	?-ln -s EDS/libEDS_dbg.a ../libEDS_dbg.a
	?-touch -r libEDS_dbg.a ../libEDS_dbg.a

clear::
	?-rm -rf $(OUTPUT_DIR)/*

clear-all:	clear
	?-rm -f ../libEDS_dbg.a libEDS_dbg.a

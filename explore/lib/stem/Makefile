# Time-stamp: <99/09/07 17:40:09 ptr>
# $SunId$ %Q%

PWD :sh = pwd
BASEDIR = $(PWD:%/lib/EDS=%)

default:	libdbg_so
libdbg_so:	../libEDS_dbg.so
libdbg_a:	../libEDS_dbg.a

include ../../Makefile.inc

INCLUDES += -I../../include/mt \
	    -I../../include/sockios \
	    -I../../include/EDS

EDS_SRC = _EventHandler.cc NetTransport.cc EvManager.cc EvPack.cc crc.cc \
          _SessionMgr.cc Names.cc
EDS_OBJ = $(EDS_SRC:%.cc=$(OUTPUT_DIR)/%.o)

libdbg_so := LDFLAGS += -L..
libdbg_so := LDLIBS += -lsockios_dbg -lxmt_dbg

../libEDS_dbg.a:	$(OUTPUT_DIR)/libEDS_dbg.a
	?@if [ ! -f ../libEDS_dbg.a ] ; then \
	  ln $(OUTPUT_DIR)/libEDS_dbg.a ../libEDS_dbg.a ; \
	else \
	  rm ../libEDS_dbg.a ; \
	  ln $(OUTPUT_DIR)/libEDS_dbg.a ../libEDS_dbg.a ; \
	fi

../libEDS_dbg.so:	$(OUTPUT_DIR)/libEDS_dbg.so
	?@if [ ! -f ../libEDS_dbg.so ] ; then \
	  ln $(OUTPUT_DIR)/libEDS_dbg.so ../libEDS_dbg.so ; \
	else \
	  rm ../libEDS_dbg.so ; \
	  ln $(OUTPUT_DIR)/libEDS_dbg.so ../libEDS_dbg.so ; \
	fi

$(OUTPUT_DIR)/libEDS_dbg.a:	$(EDS_OBJ)
	$(LINK.cc) $(OUTPUT_OPTION) $(EDS_OBJ) $(LDLIBS)

$(OUTPUT_DIR)/libEDS_dbg.so:	$(EDS_OBJ)
	$(LINK.cc) $(OUTPUT_OPTION) $(EDS_OBJ) $(LDLIBS)

clear::
	?-rm -rf $(OUTPUT_DIR)/*

clear-all:	clear
	?-rm -f ../libEDS_dbg.a
	?-rm -f ../libEDS_dbg.so

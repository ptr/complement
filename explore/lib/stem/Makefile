# %Z% $Date$ $Revision$ $RCSfile$ %Q%
# Implementation and debugging OXW (Object X Window) class library (Release 2)
# Time-stamp: <96/10/09 17:20:28 ptr>

PROJECT = ../../libEDS.so

INCLUDES = \
	-I/usr/openwin/include \
	-I../../include \
	-I../../../../STL.labs

DEFS = \
	-DSVR4 \
	-D__DEBUG=2 \
	-D__TRACE -D__WARN
# OPT = -g -p
OPT = -g
# OPT = -O4
# OPT = -G -O4

# CCFLAGS = $(OPT) -cg92 +w -PIC -instances=static -mt
CCFLAGS = $(OPT) +w -PIC -instances=static -mt -features=no%anachronisms \
	-features=rtti
CPPFLAGS = $(DEFS) $(INCLUDES)
PROJECT_LIBS = -L../.. -L../../../../STL.labs -lCLASS -lSTL
# LDFLAGS = $(PROJECT_LIBS) -lX11 -lXpm -ldl
LDFLAGS = $(PROJECT_LIBS)

INSTALL_LIB_DIR = /usr/home/ptr/lib
OUTPUT_DIR = ./bin
OUTPUT_TESTS_DIR = ./tests/bin
OUTPUT_OPTION = -o $@

TESTS_DIR = ./tests

# Library files

ELIBFILES.o = \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/EvManager.o \
		$(OUTPUT_DIR)/Application.o

LIBFILES.o = $(ELIBFILES.o)

.KEEP_STATE:

.PARALLEL:

all:	$(OUTPUT_DIR) $(PROJECT)

$(PROJECT):	$(LIBFILES.o)
	$(LINK.cc) -G -o $@ $(LIBFILES.o)

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_TESTS_DIR)/%.o:	$(TESTS_DIR)/%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR)::
	@if [ ! -d $(OUTPUT_DIR) ]; \
	then \
	  mkdir $(OUTPUT_DIR); \
	fi

$(OUTPUT_TESTS_DIR)::
	@if [ ! -d $(OUTPUT_TESTS_DIR) ]; \
	then \
	  mkdir $(OUTPUT_TESTS_DIR); \
	fi

clear:
	@if [ -d $(OUTPUT_DIR) ]; \
	then \
	  rm -rf $(OUTPUT_DIR); \
	fi
	@if [ -d $(OUTPUT_TESTS_DIR) ]; \
	then \
	  rm -rf $(OUTPUT_TESTS_DIR); \
	fi

clearall:	clear
	rm -rf $(PROJECT)

install:	all	
	@cp -p $(PROJECT) $(INSTALL_LIB_DIR)

# Time-stamp: <96/10/16 13:23:38 ptr>
# %Z% $Date$ $Revision$ $RCSfile$ %Q%
# Implementation and debugging EDS (Event-Driven System) class library
# (Release 2), derived from OXW project: EDS is parent library for OXW.

PROJECT = ./libEDS.so

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

# CCFLAGS = $(OPT) -cg92 +w -PIC -instances=static -mt
CCFLAGS = $(OPT) +w -PIC -instances=static -mt -features=no%anachronisms \
	-features=rtti
CPPFLAGS = $(DEFS) $(INCLUDES)
PROJECT_LIBS = -L$(INSTALL_LIB_DIR) -lCLASS -lSTL
LDFLAGS = $(PROJECT_LIBS)

INSTALL_LIB_DIR = /usr/home/ptr/lib
INSTALL_CMD = /usr/home/ptr/bin/install-sh
OUTPUT_DIR = ./bin
OUTPUT_TESTS_DIR = ./tests/bin
OUTPUT_OPTION = -o $@

TESTS_DIR = ./tests

# Library files

LIBFILES.o = \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/EvManager.o \
		$(OUTPUT_DIR)/Application.o

.KEEP_STATE:

.PARALLEL:

all:	dir $(PROJECT)

$(PROJECT):	$(LIBFILES.o)
	$(LINK.cc) -G -o $@ $(LIBFILES.o)

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_TESTS_DIR)/%.o:	$(TESTS_DIR)/%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

dir::	$(OUTPUT_DIR)

$(OUTPUT_DIR)::
	@if [ ! -d $(OUTPUT_DIR) ]; \
	then \
	  mkdir $(OUTPUT_DIR); \
	fi

clear::
	@if [ -d $(OUTPUT_DIR) ]; \
	then \
	  rm -f $(OUTPUT_DIR)/*.o; \
	fi

dir::	$(OUTPUT_TESTS_DIR)

$(OUTPUT_TESTS_DIR)::
	@if [ ! -d $(OUTPUT_TESTS_DIR) ]; \
	then \
	  mkdir $(OUTPUT_TESTS_DIR); \
	fi

clear::
	@if [ -d $(OUTPUT_TESTS_DIR) ]; \
	then \
	  rm -f $(OUTPUT_TESTS_DIR)/*.o; \
	fi
	@rm core *~;

clearall:	clear
	rm $(PROJECT)

install::	$(INSTALL_LIB_DIR)/$(PROJECT)

uninstall::	clearall
	rm -f $(INSTALL_LIB_DIR)/$(PROJECT)

$(INSTALL_LIB_DIR)/$(PROJECT):	$(PROJECT)
	$(INSTALL_CMD) -c $(PROJECT) $(INSTALL_LIB_DIR)


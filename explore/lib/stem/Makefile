# Time-stamp: <97/09/26 16:57:33 ptr>
# %Z% $Date$ $Revision$ $RCSfile$ %Q%
# Implementation and debugging EDS class library.

baselibname = EDS
libversion = 2
libname        = $(baselibname:%=lib%.so)
libnamedebug   = $(baselibname:%=lib%d.so)
libname_v      = $(libname:%=%.$(libversion))
libnamedebug_v = $(libnamedebug:%=%.$(libversion))

INCLUDES = \
	-I../../include \
	-I/usr/local/include/STL

INSTALL_LIB_DIR = /usr/home/ptr/lib
LDFLAGS = 
DEFS =
$(libnamedebug_v) := DEFS += -D__DEBUG=2 -D__TRACE -D__WARN
$(libname_v)      := OPT = -O -pic
$(libnamedebug_v) := OPT = -g -pic +w
$(libname_v)      := LDFLAGS += -lCLASS
$(libnamedebug_v) := LDFLAGS += -lCLASSd

CCFLAGS = $(OPT) -instances=static -mt -features=no%anachronisms -features=rtti
CPPFLAGS = $(DEFS) $(INCLUDES)

INSTALL_CMD = install-sh
OUTPUT_DIR = ./bin
OUTPUT_OPTION = -o $@

# Library files

%:	%.c

LIBFILES = \
	EventsCore.cc \
	EventHandler.cc \
	EvManager.cc \
	Application.cc

LIBFILES.o = $(LIBFILES:%.cc=$(OUTPUT_DIR)/%.o)

.KEEP_STATE:

.PARALLEL:

$(libname_v) $(libnamedebug_v):	$(LIBFILES.o)
	$(LINK.cc) -G -o $@ -h `basename $@` $(LIBFILES.o)

$(libname):	$(libname_v)
	-@rm -f $(libname)
	-@ln -s $(libname_v) $(libname)

$(libnamedebug):	$(libnamedebug_v)
	-@rm -f $(libnamedebug)
	-@ln -s $(libnamedebug_v) $(libnamedebug)

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

dir:	$(OUTPUT_DIR)

$(OUTPUT_DIR):
	-@if [ ! -d $(OUTPUT_DIR) ]; \
	then \
	  mkdir $(OUTPUT_DIR); \
	fi

clear:
	-@if [ -d $(OUTPUT_DIR) ]; \
	then \
	  rm -f $(LIBFILES.o); \
	fi

clearall:	clear
	-@rm -f $(libname) $(libnamedebug) $(libname_v) $(libnamedebug_v) 

install:
	-$(INSTALL_CMD) -m 0755 -c $(libname_v)      $(INSTALL_LIB_DIR)
	-@(cd $(INSTALL_LIB_DIR); \
	   rm -f $(libname); \
           ln -s $(libname_v) $(libname) )
	-$(INSTALL_CMD) -m 0755 -c $(libnamedebug_v) $(INSTALL_LIB_DIR)
	-@(cd $(INSTALL_LIB_DIR); \
           rm -f $(libnamedebug); \
           ln -s $(libnamedebug_v) $(libnamedebug) )

# %Z% $Date$ $Revision$ $RCSfile$ %Q%
# Implementation and debugging OXW (Object X Window) class library (Release 2)
# Time-stamp: <96/03/04 10:28:39 ptr>

PROJECT = ../../libOXW.so
MENU0 = menu0
MENU1 = menu1
CONTROLS0 = controls0
CONTAINER0 = container0
CONTAINER1 = container1
ACTION0 = action0
NOTES0 = bookmarks0
OPERIN = OperIn

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

CCFLAGS = $(OPT) -cg92 +w -PIC -pto -mt
CPPFLAGS = $(DEFS) $(INCLUDES)
PROJECT_LIBS = -xildoff -L../.. -L../../../../STL.labs -lCLASS -lSTL
# LDFLAGS = $(PROJECT_LIBS) -lX11 -lXpm -ldl
LDFLAGS = $(PROJECT_LIBS) -lX11 -lXpm -lXext

OUTPUT_DIR = /usr/home/ptr/tmp
OUTPUT_TESTS_DIR = /usr/home/ptr/tmp
OUTPUT_OPTION = -o $@

TESTS_DIR = ./tests

# Library files

XLIBFILES.o = \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/Diagnosis.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/Actuator.o \
		$(OUTPUT_DIR)/Resource.o \
		$(OUTPUT_DIR)/EvManager.o \
		$(OUTPUT_DIR)/Application.o \
		$(OUTPUT_DIR)/RectObject.o \
		$(OUTPUT_DIR)/WindowBase.o \
		$(OUTPUT_DIR)/Misc.o \
		$(OUTPUT_DIR)/Window.o \
		$(OUTPUT_DIR)/GrWindow.o \
		$(OUTPUT_DIR)/DrawTools.o \
		$(OUTPUT_DIR)/WMClient.o \
		$(OUTPUT_DIR)/GrContext.o \
		$(OUTPUT_DIR)/Frame.o \
		$(OUTPUT_DIR)/String.o \
		$(OUTPUT_DIR)/Label.o \
		$(OUTPUT_DIR)/Button.o \
		$(OUTPUT_DIR)/ButtonCheck.o \
		$(OUTPUT_DIR)/ButtonPic.o \
		$(OUTPUT_DIR)/Input.o \
		$(OUTPUT_DIR)/PopupMenu.o \
		$(OUTPUT_DIR)/Menubar.o \
		$(OUTPUT_DIR)/Picture.o \
		$(OUTPUT_DIR)/Bookmarks.o

# test program

MENU0.o = \
		$(OUTPUT_TESTS_DIR)/menu0.o \
		$(OUTPUT_TESTS_DIR)/menu0main.o

MENU1.o = \
		$(OUTPUT_TESTS_DIR)/menu1.o \
		$(OUTPUT_TESTS_DIR)/menu1main.o

CONTROLS0.o = \
		$(OUTPUT_TESTS_DIR)/controls0.o \
		$(OUTPUT_TESTS_DIR)/controls0main.o

CONTAINER0.o = \
		$(OUTPUT_TESTS_DIR)/container0.o \
		$(OUTPUT_TESTS_DIR)/container0main.o

CONTAINER1.o = \
		$(OUTPUT_TESTS_DIR)/container1.o \
		$(OUTPUT_TESTS_DIR)/container1main.o

ACTION0.o = \
		$(OUTPUT_TESTS_DIR)/action0.o \
		$(OUTPUT_TESTS_DIR)/action0main.o

NOTES0.o = \
		$(OUTPUT_TESTS_DIR)/bookmarks0.o \
		$(OUTPUT_TESTS_DIR)/bookmarks0main.o

OPERIN.o = \
		$(OUTPUT_TESTS_DIR)/OperIn.o \
		$(OUTPUT_TESTS_DIR)/OperInMain.o

LIBFILES.o = $(XLIBFILES.o)

.KEEP_STATE:

.PARALLEL:

all:	$(OUTPUT_DIR) $(PROJECT)

$(PROJECT):	$(LIBFILES.o)
	$(LINK.cc) -G -o $@ $(LIBFILES.o)

$(MENU0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(MENU0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(MENU0.o)

$(MENU1):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(MENU1.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(MENU1.o)

$(CONTROLS0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(CONTROLS0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(CONTROLS0.o)

$(CONTAINER0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(CONTAINER0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(CONTAINER0.o)

$(CONTAINER1):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(CONTAINER1.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(CONTAINER1.o)

$(ACTION0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(ACTION0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(ACTION0.o)

$(NOTES0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(NOTES0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(NOTES0.o)

$(OPERIN):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(OPERIN.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(OPERIN.o)

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

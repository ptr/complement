# "%Z%$RCSfile$ v.$Revision$ %H% %T%"
# Makefile for implementation and debugging OXW (Object X Window) class library

INCLUDES = -I/usr/openwin/include -I.
DEFS = -DSVR4 -D__DEBUG=2
# OPT = -g -p
OPT = -g
CCFLAGS = $(OPT) -cg92 +w -pic -pto -mt
CPPFLAGS = $(DEFS) $(INCLUDES)
# LDFLAGS = -L$(OUTPUT_DIR) -lclass -lX11 -ldl
LDFLAGS = -L$(OUTPUT_DIR) -lclass -lX11 -lXpm
OUTPUT_DIR = ./bin
OXW_DIR = ./OXW
OUTPUT_OPTION = -o $@

# Library files

XLIBFILES.o = \
		$(OUTPUT_DIR)/Event.o \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/Application.o \
		$(OUTPUT_DIR)/RectObject.o \
		$(OUTPUT_DIR)/WindowBase.o \
		$(OUTPUT_DIR)/Window.o \
		$(OUTPUT_DIR)/Resource.o \
		$(OUTPUT_DIR)/DrawTools.o \
		$(OUTPUT_DIR)/TopLevel.o \
		$(OUTPUT_DIR)/GrContext.o \
		$(OUTPUT_DIR)/Frame.o \
		$(OUTPUT_DIR)/Button.o \
		$(OUTPUT_DIR)/varstring.o \
		$(OUTPUT_DIR)/TextLabel.o \
		$(OUTPUT_DIR)/InputField.o \
		$(OUTPUT_DIR)/ScrWindow.o \
		$(OUTPUT_DIR)/PopupMenu.o \
		$(OUTPUT_DIR)/AlignUtils.o \
		$(OUTPUT_DIR)/CheckList.o \
		$(OUTPUT_DIR)/Scroller.o \
		$(OUTPUT_DIR)/TagetUtils.o \
		$(OUTPUT_DIR)/TableInfo.o


# Xlib-based project

XTESTER = xtester
XFILES.o = \
		$(OUTPUT_DIR)/xtester.o \
		$(OUTPUT_DIR)/xmain.o


.KEEP_STATE:

.PARALLEL:

all:	$(PROJECT)

$(PROJECT):	$(LIBFILES.o) $(PROJECTFILES.o)
	$(LINK.cc) -o $@ $(LIBFILES.o) $(PROJECTFILES.o)

$(XTESTER):	$(XLIBFILES.o) $(XFILES.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(XFILES.o)

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

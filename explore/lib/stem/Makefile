# "%Z%$RCSfile$ v.$Revision$ %H% %T%"
# Makefile for implementation and debugging OXW (Object X Window) class library

INCLUDES = -I/usr/openwin/include -I../../../include
DEFS = -DSVR4 -D__DEBUG=2
# OPT = -g -p
OPT = -g
# OPT = -O4
# OPT = -G -O4
CCFLAGS = $(OPT) -cg92 +w -pic -pto -mt
CPPFLAGS = $(DEFS) $(INCLUDES)
# LDFLAGS = -L$(OUTPUT_DIR) -lclass -lX11 -ldl
PROJECT_LIBS = ../..
LDFLAGS = -L$(PROJECT_LIBS) -lclass -lX11 -lXpm
OUTPUT_DIR = ./bin
OUTPUT_OPTION = -o $@

# Library files

XLIBFILES.o = \
		$(OUTPUT_DIR)/Event.o \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/Actuator.o \
		$(OUTPUT_DIR)/Application.o \
		$(OUTPUT_DIR)/RectObject.o \
		$(OUTPUT_DIR)/WindowBase.o \
		$(OUTPUT_DIR)/Window.o \
		$(OUTPUT_DIR)/Chameleon.o \
		$(OUTPUT_DIR)/Collector.o \
		$(OUTPUT_DIR)/Resource.o \
		$(OUTPUT_DIR)/DrawTools.o \
		$(OUTPUT_DIR)/TopLevel.o \
		$(OUTPUT_DIR)/GrContext.o \
		$(OUTPUT_DIR)/Frame.o \
		$(OUTPUT_DIR)/Button.o \
		$(OUTPUT_DIR)/ButtonedInfo.o \
		$(OUTPUT_DIR)/CoupleButtons.o \
		$(OUTPUT_DIR)/varstring.o \
		$(OUTPUT_DIR)/TextLabel.o \
		$(OUTPUT_DIR)/RubberTextLabel.o \
		$(OUTPUT_DIR)/InputField.o \
		$(OUTPUT_DIR)/TextCursor.o \
		$(OUTPUT_DIR)/ScrVCollector.o \
		$(OUTPUT_DIR)/ScrWindow.o \
		$(OUTPUT_DIR)/ScrTable.o \
		$(OUTPUT_DIR)/SpeedyButton.o \
		$(OUTPUT_DIR)/PopupMenu.o \
		$(OUTPUT_DIR)/CheckList.o \
		$(OUTPUT_DIR)/Scroller.o \
		$(OUTPUT_DIR)/TargetUtils.o \
		$(OUTPUT_DIR)/TableInfo.o \
		$(OUTPUT_DIR)/CheckBox.o \
		$(OUTPUT_DIR)/ControlData.o \
		$(OUTPUT_DIR)/Filter.o \
		$(OUTPUT_DIR)/PanelItem.o \
		$(OUTPUT_DIR)/Panel.o \
		$(OUTPUT_DIR)/ModalDialog.o


# Xlib-based project

XTESTER = xtester
XFILES.o = \
		$(OUTPUT_DIR)/xtester.o \
		$(OUTPUT_DIR)/xmain.o
PROJECT = ../../libOXW.so
LIBFILES.o = $(XLIBFILES.o)

.KEEP_STATE:

.PARALLEL:

all:	$(PROJECT)

$(PROJECT):	$(LIBFILES.o)
	$(LINK.cc) -G -o $@ $(LIBFILES.o)

$(XTESTER):	$(XLIBFILES.o) $(XFILES.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(XFILES.o)

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

# "%Z%$RCSfile$ v.$Revision$ %H% %T%"
# Makefile for implementation and debugging OXW (Object X Window) class library

#	-I/usr/hostel/STL/include \

INCLUDES = -I/usr/openwin/include \
	   -I../../../include

DEFS = \
	-DSVR4 \
	-D__DEBUG=2
# OPT = -g -p
OPT = -g
# OPT = -O4
# OPT = -G -O4

CCFLAGS = $(OPT) -cg92 +w -pic -pto -mt
CPPFLAGS = $(DEFS) $(INCLUDES)
PROJECT_LIBS = -L../.. -lclass
# LDFLAGS = $(PROJECT_LIBS) -lX11 -lXpm -ldl
LDFLAGS =  -xildoff $(PROJECT_LIBS) -lX11 -lXpm

OUTPUT_DIR = ./bin
OUTPUT_TESTS_DIR = ./tests-bin
OUTPUT_OPTION = -o $@

TESTS_DIR = ./tests

# Library files

XLIBFILES.o = \
		$(OUTPUT_DIR)/Event.o \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/Actuator.o \
		$(OUTPUT_DIR)/Resource.o \
		$(OUTPUT_DIR)/Application.o \
		$(OUTPUT_DIR)/RectObject.o \
		$(OUTPUT_DIR)/WindowBase.o \
		$(OUTPUT_DIR)/Window.o \
		$(OUTPUT_DIR)/Collector.o \
		$(OUTPUT_DIR)/DrawTools.o \
		$(OUTPUT_DIR)/TopLevel.o \
		$(OUTPUT_DIR)/GrContext.o \
		$(OUTPUT_DIR)/Frame.o \
		$(OUTPUT_DIR)/Button.o \
		$(OUTPUT_DIR)/ButtonedInfo.o \
		$(OUTPUT_DIR)/ButtonedInfoPI.o \
		$(OUTPUT_DIR)/ButtonedCheckbox.o \
		$(OUTPUT_DIR)/CoupleButtons.o \
		$(OUTPUT_DIR)/varstring.o \
		$(OUTPUT_DIR)/TextLabel.o \
		$(OUTPUT_DIR)/RubberTextLabel.o \
		$(OUTPUT_DIR)/InputField.o \
		$(OUTPUT_DIR)/InputMoney.o \
		$(OUTPUT_DIR)/InputDate.o \
		$(OUTPUT_DIR)/TextFilter.o \
		$(OUTPUT_DIR)/TextCursor.o \
		$(OUTPUT_DIR)/ScrVCollector.o \
		$(OUTPUT_DIR)/ScrWindow.o \
		$(OUTPUT_DIR)/ScrTable.o \
		$(OUTPUT_DIR)/SpeedyButton.o \
		$(OUTPUT_DIR)/PopupMenu.o \
		$(OUTPUT_DIR)/Menubar.o \
		$(OUTPUT_DIR)/CheckList.o \
		$(OUTPUT_DIR)/Scroller.o \
		$(OUTPUT_DIR)/TableInfo.o \
		$(OUTPUT_DIR)/CheckBox.o \
		$(OUTPUT_DIR)/ControlData.o \
		$(OUTPUT_DIR)/Filter.o \
		$(OUTPUT_DIR)/PanelItem.o \
		$(OUTPUT_DIR)/Panel.o \
		$(OUTPUT_DIR)/ANPanelItem.o \
		$(OUTPUT_DIR)/ANPanel.o \
		$(OUTPUT_DIR)/CheckBoxPI.o \
		$(OUTPUT_DIR)/Matrix.o \
		$(OUTPUT_DIR)/ModalDialog.o \
		$(OUTPUT_DIR)/AlertPanel.o \
		$(OUTPUT_DIR)/Picture.o \
		$(OUTPUT_DIR)/ModalDialogBase.o


# Xlib-based project

XTESTER = xtester
XFILES.o = \
		$(OUTPUT_TESTS_DIR)/xtester.o \
		$(OUTPUT_TESTS_DIR)/xmain.o
PROJECT = ../../libOXW.so
LIBFILES.o = $(XLIBFILES.o)

.KEEP_STATE:

.PARALLEL:

all:	dirs $(PROJECT)

dirs:
	@if [ ! -d ./bin ]; \
	then \
	  mkdir ./bin; \
	fi
tests-dirs:
	@if [ ! -d ./tests-bin ]; \
	then \
	  mkdir ./tests-bin; \
	fi

$(PROJECT):	$(LIBFILES.o)
	$(LINK.cc) -G -o $@ $(LIBFILES.o)

$(XTESTER):	$(XLIBFILES.o) $(XFILES.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(XFILES.o)

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_TESTS_DIR)/%.o:	$(TESTS_DIR)/%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

clear:
	@if [ -d ./bin ]; \
	then \
	  rm -rf ./bin; \
	fi
	@if [ -d ./tests-bin ]; \
	then \
	  rm -rf ./tests-bin; \
	fi

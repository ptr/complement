# "%Z%$RCSfile$ v.$Revision$ %H% %T%"
# Makefile for implementation and debugging OXW (Object X Window) class library

PROJECT = ../../libOXW.so
XTESTER = xtester
YTESTER = ytester
TONE = tester1
TTWO = tester2

INCLUDES = \
	-I/usr/openwin/include \
	-I../../../include \
	-I../../../../STL/include

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
PROJECT_LIBS = -L../.. -L../../../../STL -lCLASS -lSTL
# LDFLAGS = $(PROJECT_LIBS) -lX11 -lXpm -ldl
LDFLAGS = $(PROJECT_LIBS) -lX11 -lXpm 

OUTPUT_DIR = ./bin
OUTPUT_TESTS_DIR = ./tests-bin
OUTPUT_OPTION = -o $@

TESTS_DIR = ./tests

# Library files

XLIBFILES.o = \
		$(OUTPUT_DIR)/Diagnosis.o \
		$(OUTPUT_DIR)/Event.o \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/Actuator.o \
		$(OUTPUT_DIR)/Resource.o \
		$(OUTPUT_DIR)/Application.o \
		$(OUTPUT_DIR)/RectObject.o \
		$(OUTPUT_DIR)/WindowBase.o \
		$(OUTPUT_DIR)/Window.o \
		$(OUTPUT_DIR)/GrWindow.o \
		$(OUTPUT_DIR)/Collector.o \
		$(OUTPUT_DIR)/DrawTools.o \
		$(OUTPUT_DIR)/TopLevel.o \
		$(OUTPUT_DIR)/GrContext.o \
		$(OUTPUT_DIR)/Frame.o \
		$(OUTPUT_DIR)/Button.o \
		$(OUTPUT_DIR)/ButtonedInfo.o \
		$(OUTPUT_DIR)/ButtonedInfoPI.o \
		$(OUTPUT_DIR)/StrButtonedInfoPI.o \
		$(OUTPUT_DIR)/InfoField.o \
		$(OUTPUT_DIR)/InfoPIBase.o \
		$(OUTPUT_DIR)/InfoPI.o \
		$(OUTPUT_DIR)/InfoIntPI.o \
		$(OUTPUT_DIR)/InfoDatePI.o \
		$(OUTPUT_DIR)/Money.o \
		$(OUTPUT_DIR)/MoneyInfoPI.o \
		$(OUTPUT_DIR)/ChMoneyInfoPI.o \
		$(OUTPUT_DIR)/ButtonedCheckbox.o \
		$(OUTPUT_DIR)/CoupleButtons.o \
		$(OUTPUT_DIR)/varstring.o \
		$(OUTPUT_DIR)/TextLabel.o \
		$(OUTPUT_DIR)/RubberTextLabel.o \
		$(OUTPUT_DIR)/ANInputField.o \
		$(OUTPUT_DIR)/InputPIBase.o \
		$(OUTPUT_DIR)/InputPI.o \
		$(OUTPUT_DIR)/BlindInputPI.o \
		$(OUTPUT_DIR)/InputNumPI.o \
		$(OUTPUT_DIR)/InputPcPI.o \
		$(OUTPUT_DIR)/InputMoneyPI.o \
		$(OUTPUT_DIR)/ANInputMoneyPI.o \
		$(OUTPUT_DIR)/InputField.o \
		$(OUTPUT_DIR)/InputMoney.o \
		$(OUTPUT_DIR)/InputDate.o \
		$(OUTPUT_DIR)/TextFilter.o \
		$(OUTPUT_DIR)/ANTextFilter.o \
		$(OUTPUT_DIR)/TextCursor.o \
		$(OUTPUT_DIR)/ScrVCollector.o \
		$(OUTPUT_DIR)/ScrWindow.o \
		$(OUTPUT_DIR)/ScrTable.o \
		$(OUTPUT_DIR)/SpeedyButton.o \
		$(OUTPUT_DIR)/PopupMenu.o \
		$(OUTPUT_DIR)/Menubar.o \
		$(OUTPUT_DIR)/CheckList.o \
		$(OUTPUT_DIR)/CheckListPI.o \
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
		$(OUTPUT_DIR)/ButtonXPM.o \
		$(OUTPUT_DIR)/ModalDialogBase.o \
		$(OUTPUT_DIR)/CAPPictures.o \
		\
		$(OUTPUT_DIR)/CAPPropertyTypePI.o \
		$(OUTPUT_DIR)/CAPCheckListPI.o \
		$(OUTPUT_DIR)/ComplexInput.o \
		$(OUTPUT_DIR)/InputUnit.o


# test program

XFILES.o = \
		$(OUTPUT_TESTS_DIR)/xtester.o \
		$(OUTPUT_TESTS_DIR)/xmain.o

YFILES.o = \
		$(OUTPUT_TESTS_DIR)/ytester.o \
		$(OUTPUT_TESTS_DIR)/ymain.o

TONE.o = \
		$(OUTPUT_TESTS_DIR)/tester1.o \
		$(OUTPUT_TESTS_DIR)/tester1main.o

TTWO.o = \
		$(OUTPUT_TESTS_DIR)/tester2.o \
		$(OUTPUT_TESTS_DIR)/tester2main.o


LIBFILES.o = $(XLIBFILES.o)

.KEEP_STATE:

.PARALLEL:

all:	$(OUTPUT_DIR) $(PROJECT)

$(PROJECT):	$(LIBFILES.o)
	$(LINK.cc) -G -o $@ $(LIBFILES.o)

$(XTESTER):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(XFILES.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(XFILES.o)

$(YTESTER):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(YFILES.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(YFILES.o)

$(TONE):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TONE.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TONE.o)

$(TTWO):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TTWO.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TTWO.o)

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_TESTS_DIR)/%.o:	$(TESTS_DIR)/%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

$(OUTPUT_DIR):
	@if [ ! -d $(OUTPUT_DIR) ]; \
	then \
	  mkdir $(OUTPUT_DIR); \
	fi

$(OUTPUT_TESTS_DIR):
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

# "%Z%%Q%$RCSfile$, ($Revision$) %H% %T%"
# Makefile for implementation and debugging OXW (Object X Window) class library

PROJECT = ../../libOXW.so
XTESTER = xtester
YTESTER = ytester
TONE = tester1
TTWO = tester2
TMENU0 = menu0
TMENU1 = menu1
TCONTROLS0 = controls0
TCONTAINER0 = container0
TCONTAINER1 = container1
TACTION0 = action0
TNOTES0 = bookmarks0
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

CCFLAGS = $(OPT) -cg92 +w -PIC -pto
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
		$(OUTPUT_DIR)/Diagnosis.o \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/Dispatch.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/Actuator.o \
		$(OUTPUT_DIR)/Resource.o \
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


#		$(OUTPUT_DIR)/TextLabel.o \
#		$(OUTPUT_DIR)/TextFilter.o
#		$(OUTPUT_DIR)/CheckList.o \
#		$(OUTPUT_DIR)/CheckBox.o \
#		$(OUTPUT_DIR)/ControlData.o \
#		$(OUTPUT_DIR)/ButtonXPM.o
#		$(OUTPUT_DIR)/InputField.o \
#		$(OUTPUT_DIR)/TextCursor.o \
#		$(OUTPUT_DIR)/Filter.o \
#		$(OUTPUT_DIR)/Scroller.o \
#		$(OUTPUT_DIR)/ScrVCollector.o \
#		$(OUTPUT_DIR)/ScrWindow.o \
#		$(OUTPUT_DIR)/ScrTable.o \
#		 $(OUTPUT_DIR)/CheckListPI.o \
#		 $(OUTPUT_DIR)/SpeedyButton.o \
#		 $(OUTPUT_DIR)/Matrix.o \
#		 $(OUTPUT_DIR)/ModalDialog.o \
#		 $(OUTPUT_DIR)/Money.o \
#		 $(OUTPUT_DIR)/MoneyInfoPI.o \
#		 $(OUTPUT_DIR)/ChMoneyInfoPI.o \
#		 $(OUTPUT_DIR)/ButtonedInfo.o \
#		 $(OUTPUT_DIR)/ButtonedInfoPI.o \
#		 $(OUTPUT_DIR)/StrButtonedInfoPI.o \
#		 $(OUTPUT_DIR)/InfoField.o \
#		 $(OUTPUT_DIR)/InfoPIBase.o \
#		 $(OUTPUT_DIR)/InfoPI.o \
#		 $(OUTPUT_DIR)/InfoIntPI.o \
#		 $(OUTPUT_DIR)/InfoDatePI.o \
#		 $(OUTPUT_DIR)/ButtonedCheckbox.o \
#		 $(OUTPUT_DIR)/CoupleButtons.o \
#		 $(OUTPUT_DIR)/ANTextFilter.o \
#		 $(OUTPUT_DIR)/InputMoney.o \
#		 $(OUTPUT_DIR)/InputDate.o \
#		 $(OUTPUT_DIR)/RubberTextLabel.o \
#		 $(OUTPUT_DIR)/ANInputField.o \
#		 $(OUTPUT_DIR)/InputPIBase.o \
#		 $(OUTPUT_DIR)/InputPI.o \
#		 $(OUTPUT_DIR)/BlindInputPI.o \
#		 $(OUTPUT_DIR)/InputNumPI.o \
#		 $(OUTPUT_DIR)/InputMoneyPI.o \
#		 $(OUTPUT_DIR)/ANInputMoneyPI.o \
#		 $(OUTPUT_DIR)/TableInfo.o \
#		 $(OUTPUT_DIR)/PanelItem.o \
#		 $(OUTPUT_DIR)/Panel.o \
#		 $(OUTPUT_DIR)/ANPanelItem.o \
#		 $(OUTPUT_DIR)/ANPanel.o \
#		 $(OUTPUT_DIR)/CheckBoxPI.o \
#		 $(OUTPUT_DIR)/AlertPanel.o \
#		 $(OUTPUT_DIR)/ModalDialogBase.o \
#		 $(OUTPUT_DIR)/CAPPropertyTypePI.o \
#		 $(OUTPUT_DIR)/CAPCheckListPI.o \
#		 $(OUTPUT_DIR)/ComplexInput.o \
#		 $(OUTPUT_DIR)/InputUnit.o
#		 $(OUTPUT_DIR)/CAPPictures.o \


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

TMENU0.o = \
		$(OUTPUT_TESTS_DIR)/menu0.o \
		$(OUTPUT_TESTS_DIR)/menu0main.o

TMENU1.o = \
		$(OUTPUT_TESTS_DIR)/menu1.o \
		$(OUTPUT_TESTS_DIR)/menu1main.o

TCONTROLS0.o = \
		$(OUTPUT_TESTS_DIR)/controls0.o \
		$(OUTPUT_TESTS_DIR)/controls0main.o

TCONTAINER0.o = \
		$(OUTPUT_TESTS_DIR)/container0.o \
		$(OUTPUT_TESTS_DIR)/container0main.o

TCONTAINER1.o = \
		$(OUTPUT_TESTS_DIR)/container1.o \
		$(OUTPUT_TESTS_DIR)/container1main.o

TACTION0.o = \
		$(OUTPUT_TESTS_DIR)/action0.o \
		$(OUTPUT_TESTS_DIR)/action0main.o

TNOTES0.o = \
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

$(XTESTER):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(XFILES.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(XFILES.o)

$(YTESTER):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(YFILES.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(YFILES.o)

$(TONE):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TONE.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TONE.o)

$(TTWO):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TTWO.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TTWO.o)

$(TMENU0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TMENU0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TMENU0.o)

$(TMENU1):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TMENU1.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TMENU1.o)

$(TCONTROLS0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TCONTROLS0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TCONTROLS0.o)

$(TCONTAINER0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TCONTAINER0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TCONTAINER0.o)

$(TCONTAINER1):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TCONTAINER1.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TCONTAINER1.o)

$(TACTION0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TACTION0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TACTION0.o)

$(TNOTES0):	$(OUTPUT_DIR) $(OUTPUT_TESTS_DIR) $(XLIBFILES.o) $(TNOTES0.o)
	$(LINK.cc) -o $@ $(XLIBFILES.o) $(TNOTES0.o)

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

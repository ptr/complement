# "%Z%$RCSfile$ v.$Revision$ %H% %T%"
# Makefile for implementation and debugging OXW (Object X Window) class library

INCLUDES = -I/usr/openwin/include -I.
DEFS = -DSVR4 -D__DEBUG=2
OPT = -g -p
CCFLAGS = $(OPT) -cg92 +w -pic -pto
CPPFLAGS = $(DEFS) $(INCLUDES)
LDFLAGS = $(OUTPUT_DIR)/classlib.so -lXol -lXt -lX11 -lintl -lgen -ldl
OUTPUT_DIR = ./bin
OXW_DIR = ./OXW
OUTPUT_OPTION = -o $@

# Library files

LIBFILES.o = \
		$(OUTPUT_DIR)/Event.o \
		$(OUTPUT_DIR)/EventHandler.o \
		$(OUTPUT_DIR)/EventsCore.o \
		$(OUTPUT_DIR)/Application.o \
		$(OUTPUT_DIR)/RectObject.o \
		$(OUTPUT_DIR)/varstring.o \
		$(OUTPUT_DIR)/GrContext.o \
		$(OUTPUT_DIR)/Window.o \
		$(OUTPUT_DIR)/Collector.o \
		$(OUTPUT_DIR)/Menu.o \
		$(OUTPUT_DIR)/Button.o \
		$(OUTPUT_DIR)/Frame.o \
		$(OUTPUT_DIR)/Decor.o \
		$(OUTPUT_DIR)/ConformBttns.o \
		$(OUTPUT_DIR)/NButton.o \
		$(OUTPUT_DIR)/TextLabel.o \
		$(OUTPUT_DIR)/FramedWnd.o \
		$(OUTPUT_DIR)/Scroller.o \
		$(OUTPUT_DIR)/TextWnd.o \
		$(OUTPUT_DIR)/HeadedWnd.o \
		$(OUTPUT_DIR)/InputField.o

# Tester

PROJECT = olitIntro
PROJECTFILES.o = \
		$(OUTPUT_DIR)/wnd_tester.o \
		$(OUTPUT_DIR)/olitIntro.o

# Tester for button (and positioning)

BTESTER = bttns_tst
BFILES.o = \
		$(OUTPUT_DIR)/bttns_tester.o \
		$(OUTPUT_DIR)/lab_bttn.o

# Tester for scrolled window

STESTER = scrlist
SFILES.o = \
		$(OUTPUT_DIR)/scrlist_tester.o \
		$(OUTPUT_DIR)/scrlist.o

.KEEP_STATE:

.PARALLEL:

all:	$(PROJECT)

tst:	$(OUTPUT_DIR)/tst.o
	$(LINK.cc) -o $@ $(OUTPUT_DIR)/tst.o $(OUTPUT_DIR)/EventHandler.o

button:	$(BTESTER)

scroll:	$(STESTER)

$(PROJECT):	$(LIBFILES.o) $(PROJECTFILES.o)
	$(LINK.cc) -o $@ $(LIBFILES.o) $(PROJECTFILES.o)

$(BTESTER):	$(LIBFILES.o) $(BFILES.o)
	$(LINK.cc) -o $@ $(LIBFILES.o) $(BFILES.o)

$(STESTER):	$(LIBFILES.o) $(SFILES.o)
	$(LINK.cc) -o $@ $(LIBFILES.o) $(SFILES.o)

$(OUTPUT_DIR)/%.o:	%.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

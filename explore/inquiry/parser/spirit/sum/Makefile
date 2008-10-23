# -*- makefile -*- Time-stamp: <03/03/31 15:18:29 ptr>
# $Id$

BASEDIR := $(shell xtmp=`pwd`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; xtmp=`dirname $$xtmp`; dirname $$xtmp)
# INSTALLDIR := $(shell xtmp=`pwd`; dirname $$xtmp)
COMPILER_NAME = gcc

include ./Makefile.inc

all:	release-shared

all-shared:	release-shared dbg-shared stldbg-shared

all-debug:	dbg-shared stldbg-shared

all-release:	release-shared

include ${BASEDIR}/Makefiles/app/Makefile.inc

INCLUDES += -I$(STLPORT_INCLUDE_DIR) -I${BOOST_INCLUDE_DIR}

BUILD_SYSTEM := $(shell echo "`uname -n` `uname -s` `uname -r` `uname -v` `uname -p`")
BUILD_DATE := $(shell echo "`date +'%Y/%m/%d %T %Z'`")

ifeq ($(OS_VER),SunOS)
LDLIBS = 
else
LDLIBS = 
endif

LDLIBS += -lrt

ifeq ($(OS_VER),SunOS)
dbg-shared:	LDFLAGS += -L${INSTALL_LIB_DIR}
stldbg-shared:	LDFLAGS += -L${INSTALL_LIB_DIR}
release-shared:	LDFLAGS += -L${INSTALL_LIB_DIR}
SYSLIBS = 
endif

ifeq ($(OS_VER),Linux)
release-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR}
stldbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR}
dbg-shared:	LDFLAGS += -L${STLPORT_LIB_DIR} -L${INSTALL_LIB_DIR}
SYSLIBS = 
endif

dbg-shared : LDLIBS += \
                    -lstlport_gcc ${SYSLIBS}

stldbg-shared : LDLIBS += \
                    -lstlport_gcc_stldebug ${SYSLIBS}


release-shared : LDLIBS += \
                    -lstlport_gcc ${SYSLIBS}

$(OUTPUT_DIR)/version.o:	version.c version.h FORCE
	$(COMPILE.c) $(OUTPUT_OPTION) \
	  -DBUILD_SYSTEM="\"${BUILD_SYSTEM}\"" \
	  -DBUILD_DATE="\"${BUILD_DATE}\"" -DKEYWORD="\"${KEYWORD}\"" \
	  -DMAJOR=${MAJOR} -DMINOR=${MINOR} -DPATCH=${PATCH} version.c

$(OUTPUT_DIR_DBG)/version.o:	version.c version.h FORCE
	$(COMPILE.c) $(OUTPUT_OPTION) \
	  -DBUILD_SYSTEM="\"${BUILD_SYSTEM}\"" \
	  -DBUILD_DATE="\"${BUILD_DATE}\"" -DKEYWORD="\"${KEYWORD}\"" \
	  -DMAJOR=${MAJOR} -DMINOR=${MINOR} -DPATCH=${PATCH} version.c

$(OUTPUT_DIR_STLDBG)/version.o:	version.c version.h FORCE
	$(COMPILE.c) $(OUTPUT_OPTION) \
	  -DBUILD_SYSTEM="\"${BUILD_SYSTEM}\"" \
	  -DBUILD_DATE="\"${BUILD_DATE}\"" -DKEYWORD="\"${KEYWORD}\"" \
	  -DMAJOR=${MAJOR} -DMINOR=${MINOR} -DPATCH=${PATCH} version.c

FORCE:


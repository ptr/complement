# Time-stamp: <03/07/11 12:26:09 ptr>
# $Id$

SO := dll

ARCH := lib
AR := lib.exe
AR_INS_R := 
AR_EXTR := 
AR_OUT := /o

INSTALL := /usr/bin/install

INSTALL_SO := ${INSTALL} -c -m 0755
INSTALL_A := ${INSTALL} -c -m 0644
INSTALL_EXE := ${INSTALL} -c -m 0755

# compiler, compiler options
include ${RULESBASE}/compiler-$(COMPILER_NAME).mak


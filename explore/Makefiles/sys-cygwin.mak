# Time-stamp: <03/07/15 18:24:51 ptr>
# $Id$

# shared library:
SO  := dll
LIB := lib
EXP := exp
# executable:
EXE := .exe

# static library extention:
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


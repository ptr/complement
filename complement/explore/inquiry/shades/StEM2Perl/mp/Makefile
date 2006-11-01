# -*- Makefile -*- Time-stamp: <03/07/09 18:08:47 ptr>

SRCROOT := ../../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(SRCROOT)/include -I../stemperl

release-shared:	LDLIBS += -L../../../../build/lib -lxmt -lsockios -lstem

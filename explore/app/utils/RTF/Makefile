# -*- makefile -*- Time-stamp: <05/04/01 00:06:12 ptr>

SRCROOT := ../../../
include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

LDLIBS += -lrt -lnsl -lfl

lex.yy.cc:	rtf.lex
	flex -+ rtf.lex

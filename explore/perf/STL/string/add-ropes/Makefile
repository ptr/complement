# -*- Makefile -*- Time-stamp: <04/07/14 23:09:10 ptr>
# $Id$

DIRS = STLport-default STLport-malloc STLport-newalloc

all:
	for d in $(DIRS); do \
	  (cd $$d; make);\
	done

clean:
	for d in $(DIRS); do \
	  (cd $$d; make clean); \
	done

depend:
	for d in $(DIRS); do \
	  (cd $$d; make depend); \
	done

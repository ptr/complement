# -*- Makefile -*- Time-stamp: <03/10/12 20:35:49 ptr>
# $Id$

DIRS = default malloc newalloc

all:
	for d in $(DIRS); do \
	  (cd $$d; make); \
	done

clobber:
	for d in $(DIRS); do \
	  (cd $$d; make clobber); \
	done

clean:
	for d in $(DIRS); do \
	  (cd $$d; make clean); \
	done

depend:
	for d in $(DIRS); do \
	  (cd $$d; make depend); \
	done


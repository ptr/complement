# -*- Makefile -*- Time-stamp: <03/10/12 20:35:49 ptr>
# $Id$

DIRS = default malloc newalloc misc noshortstr

all:
	for d in $(DIRS); do \
	  (cd $$d; make); \
	done

depend:
	for d in $(DIRS); do \
	  (cd $$d; make depend); \
	done

clean:
	for d in $(DIRS); do \
	  (cd $$d; make clean); \
	done

clobber:
	for d in $(DIRS); do \
	  (cd $$d; make clobber); \
	done


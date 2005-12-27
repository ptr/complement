# -*- Makefile -*- Time-stamp: <05/12/27 00:05:49 ptr>

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


# -*- Makefile -*- Time-stamp: <04/07/14 23:10:05 ptr>
# $Id$

DIRS = STLport-default STLport-malloc STLport-newalloc

all:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE});\
	done

clean:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clean); \
	done

depend:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} depend); \
	done

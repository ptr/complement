# -*- Makefile -*- Time-stamp: <04/07/14 23:08:48 ptr>
# $Id$

DIRS = STLport-default STLport-malloc STLport-newalloc libstd++

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

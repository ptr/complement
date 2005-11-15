# -*- Makefile -*- Time-stamp: <03/10/12 20:35:49 ptr>
# $Id$

DIRS = default malloc newalloc misc noshortstr

all:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE}); \
	done

depend:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} depend); \
	done

clean:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clean); \
	done

clobber:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clobber); \
	done


# -*- Makefile -*- Time-stamp: <05/12/26 23:57:38 ptr>

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


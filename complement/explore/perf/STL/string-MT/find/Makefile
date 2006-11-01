# -*- Makefile -*- Time-stamp: <05/12/27 00:53:47 ptr>

DIRS = STLport-default STLport-malloc libstd++

all:
	for d in $(DIRS); do \
	  (cd $$d; ${make}); \
	done

clean:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clean); \
	done

clobber:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clobber); \
	done

depend:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} depend); \
	done

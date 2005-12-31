# -*- Makefile -*- Time-stamp: <05/12/30 23:51:07 ptr>

DIRS = STLport-default STLport-malloc libstd++

all:
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE}); \
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

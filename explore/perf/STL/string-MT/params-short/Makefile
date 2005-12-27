# -*- Makefile -*- Time-stamp: <05/12/27 01:05:09 ptr>

DIRS = STLport-default STLport-no-short-str libstd++

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

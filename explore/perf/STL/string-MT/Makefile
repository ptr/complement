# -*- Makefile -*- Time-stamp: <06/10/16 21:49:49 ptr>

DIRS = add add-ropes find find-ropes ops ops-ropes params params-ropes params-ref params-ref-ropes params-short add-proxy

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

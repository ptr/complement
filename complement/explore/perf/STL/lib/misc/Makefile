# -*- Makefile -*- Time-stamp: <06/01/03 17:07:22 ptr>

DIRS = default malloc newalloc libstdc++

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


# -*- Makefile -*- Time-stamp: <05/12/27 00:00:15 ptr>

DIRS := lib string string-MT stream string/ropes
TESTDIRS := string string-MT stream string/ropes

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

test:
	for d in $(TESTDIRS); do \
	  (cd $$d; ./test.sh); \
	done

.PHONY:	all depend clean clobber test


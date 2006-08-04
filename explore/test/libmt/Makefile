# -*- Makefile -*- Time-stamp: <03/07/16 21:14:45 ptr>
# $Id$

.PHONY:	all clean clobber

TESTS = delay join calendar_time \
        signal-1 signal-2 signal-3 signal-4 \
	recursive_mutex 

all:
	for d in $(TESTS); do \
	  ( cd $$d; ${MAKE} all ) || exit 1; \
	done

clean:
	for d in $(TESTS); do \
	  ( cd $$d; ${MAKE} clean ) || exit 1; \
	done

clobber:
	for d in $(TESTS); do \
	  ( cd $$d; ${MAKE} clobber ) || exit 1; \
	done

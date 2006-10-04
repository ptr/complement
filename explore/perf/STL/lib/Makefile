# -*- Makefile -*- Time-stamp: <05/12/26 23:57:38 ptr>

DIRS = default malloc newalloc misc noshortstr forcestringterm

STLPORT_SRC_DIR = ${HOME}/STLport.lab/STLport

all:	Makefile.inc
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE}); \
	done

depend:	Makefile.inc
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} depend); \
	done

clean:	Makefile.inc
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clean); \
	done

clobber:	Makefile.inc
	for d in $(DIRS); do \
	  (cd $$d; ${MAKE} clobber); \
	done
	rm -f Makefile.inc

Makefile.inc:	${STLPORT_SRC_DIR}/build/lib/Makefile.inc
	sed -e 's|\.\./\.\./src|\$${STLPORT_SRC_DIR}/src|g' ${STLPORT_SRC_DIR}/build/lib/Makefile.inc > Makefile.inc

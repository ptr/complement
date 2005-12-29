# Time-stamp: <03/08/15 12:24:23 ptr>

.PHONY:	all clean clobber install

LIBS_DIR = mt sockios stem misc

all:
	for d in $(LIBS_DIR); do \
	  ( cd $$d; ${MAKE} all ) || exit 1; \
	done

clean:
	for d in $(LIBS_DIR); do \
	  ( cd $$d; ${MAKE} clean ) || exit 1; \
	done

clobber:
	for d in $(LIBS_DIR); do \
	  ( cd $$d; ${MAKE} clobber ) || exit 1; \
	done

install:
	for d in $(LIBS_DIR); do \
	  ( cd $$d; ${MAKE} install ) || exit 1; \
	done

depend:
	for d in $(LIBS_DIR); do \
	  ( cd $$d; ${MAKE} depend ) || exit 1; \
	done

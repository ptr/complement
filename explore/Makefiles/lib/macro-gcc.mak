# -*- makefile -*- Time-stamp: <03/07/06 19:42:26 ptr>
# $Id$


# Oh, the commented below work for gmake 3.78.1 and above,
# but phrase without tag not work for it. Since gmake 3.79 
# tag with assignment fail, but work assignment for all tags
# (really that more correct).

OPT += -fPIC

ifeq ($(OSNAME),hp-ux)
dbg-shared:	LDFLAGS += -shared -Wl,-C20 -Wl,-dynamic  -Wl,+h$(SO_NAME_LINK_DBG) ${LDSEARCH}
stldbg-shared:	LDFLAGS += -shared -Wl,-C20 -Wl,-dynamic  -Wl,+h$(SO_NAME_LINK_STLDBG) ${LDSEARCH}
release-shared:	LDFLAGS += -shared -Wl,-C20 -Wl,-dynamic -Wl,+h$(SO_NAME_LINK) ${LDSEARCH}
else
dbg-shared:	LDFLAGS += -shared -Wl,-h$(SO_NAME_LINK_DBG) ${LDSEARCH}
stldbg-shared:	LDFLAGS += -shared -Wl,-h$(SO_NAME_LINK_STLDBG) ${LDSEARCH}
release-shared:	LDFLAGS += -shared -Wl,-h$(SO_NAME_LINK) ${LDSEARCH}
dbg-static:	LDFLAGS += ${LDSEARCH}
stldbg-static:	LDFLAGS += ${LDSEARCH}
release-static:	LDFLAGS += ${LDSEARCH}
endif



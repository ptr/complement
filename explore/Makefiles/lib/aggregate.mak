# -*- makefile -*- Time-stamp: <03/06/22 19:57:00 ptr>
# $Id$

include $(BASEDIR)/Makefiles/Makefile.inc
# -include $(BASEDIR)/Makefiles/Makefile.dep.${COMPILER_NAME}
include ${BASEDIR}/Makefiles/Makefile.inc.${COMPILER_NAME}
include $(BASEDIR)/Makefiles/Makefile.inc.ver
LDFLAGS = 
include ${BASEDIR}/Makefiles/lib/Makefile.inc.${COMPILER_NAME}

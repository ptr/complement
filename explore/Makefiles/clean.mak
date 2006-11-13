# -*- Makefile -*- Time-stamp: <06/11/11 01:06:53 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

PHONY += clean distclean mostlyclean maintainer-clean uninstall

clean::	
	@-rm -f core core.*
	@-rm -f $(OBJ) $(DEP)
	@-rm -f $(OBJ_DBG) $(DEP_DBG)
	@-rm -f $(OBJ_STLDBG) $(DEP_STLDBG)

distclean::	clean

mostlyclean:	clean
	@-rm -f $(DEPENDS_COLLECTION)
	@-rm -f TAGS tags

maintainer-clean:	distclean
	@rm -f ${RULESBASE}/config.mak
	@-rm -f TAGS tags

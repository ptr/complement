# -*- Makefile -*- Time-stamp: <03/07/10 13:19:47 ptr>
# $Id$

PHONY += clean clobber distclean

clean::	
	@-rm -f core core.*
	@-rm -f $(OBJ) $(DEP)
	@-rm -f $(OBJ_DBG) $(DEP_DBG)
	@-rm -f $(OBJ_STLDBG) $(DEP_STLDBG)

clobber::	clean
	@-rm -f .make.depend

distclean::	clobber

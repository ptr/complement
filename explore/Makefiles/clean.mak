# -*- Makefile -*- Time-stamp: <03/07/06 23:31:58 ptr>
# $Id$

.PHONY:	clean clobber

clean::	
	@-rm -f core core.*
	@-rm -f $(OBJ) $(DEP)
	@-rm -f $(OBJ_DBG) $(DEP_DBG)
	@-rm -f $(OBJ_STLDBG) $(DEP_STLDBG)

clobber::	clean
	@-rm .make.depend

# -*- makefile -*- Time-stamp: <03/10/26 16:13:11 ptr>
# $Id$

clean:
	@-del /F /Q $(OBJ) $(DEP)
	@-del /F /Q $(OBJ_DBG) $(DEP_DBG)
	@-del /F /Q $(OBJ_STLDBG) $(DEP_STLDBG)
	@-del /F /Q $(OBJ_A)
	@-del /F /Q $(OBJ_A_DBG)
	@-del /F /Q $(OBJ_A_STLDBG)

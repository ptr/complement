# -*- makefile -*- Time-stamp: <03/07/06 23:39:30 ptr>
# $Id$

clobber::
	@-rm -f ${SO_NAME_OUT}
	@-rm -f ${SO_NAME_OUTx}
	@-rm -f ${SO_NAME_OUTxx}
	@-rm -f ${SO_NAME_OUTxxx}
	@-rm -f ${SO_NAME_OUT_DBG}
	@-rm -f ${SO_NAME_OUT_DBGx}
	@-rm -f ${SO_NAME_OUT_DBGxx}
	@-rm -f ${SO_NAME_OUT_DBGxxx}
	@-rm -f ${SO_NAME_OUT_STLDBG}
	@-rm -f ${SO_NAME_OUT_STLDBGx}
	@-rm -f ${SO_NAME_OUT_STLDBGxx}
	@-rm -f ${SO_NAME_OUT_STLDBGxxx}

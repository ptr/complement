# Time-stamp: <03/07/07 17:15:06 ptr>
# $Id$

.PHONY:	release-static-dep release-shared-dep dbg-static-dep dbg-shared-dep \
	stldbg-static-dep stldbg-shared-dep depend

release-static-dep release-shared-dep:	$(OUTPUT_DIR) $(DEP)

dbg-static-dep dbg-shared-dep:	$(OUTPUT_DIR_DBG) $(DEP_DBG)

stldbg-static-dep stldbg-shared-dep:	$(OUTPUT_DIR_STLDBG) $(DEP_STLDBG)

depend:	release-shared-dep dbg-shared-dep stldbg-shared-dep
	cat -s $(DEP) $(DEP_DBG) $(DEP_STLDBG) /dev/null > .make.depend

-include .make.depend

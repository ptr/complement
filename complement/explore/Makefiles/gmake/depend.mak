# Time-stamp: <06/08/24 10:09:17 ptr>

PHONY += release-static-dep release-shared-dep dbg-static-dep dbg-shared-dep \
         stldbg-static-dep stldbg-shared-dep depend

release-static-dep release-shared-dep:	$(DEP)

dbg-static-dep dbg-shared-dep:	$(DEP_DBG)

stldbg-static-dep stldbg-shared-dep:	$(DEP_STLDBG)

depend:	$(OUTPUT_DIRS) release-shared-dep dbg-shared-dep stldbg-shared-dep
	@cat -s $(DEP) $(DEP_DBG) $(DEP_STLDBG) /dev/null > $(DEPENDS_COLLECTION)

TAGS:	$(OUTPUT_DIRS) release-shared-dep dbg-shared-dep stldbg-shared-dep
	@etags -i -m `cat -s $(DEP) $(DEP_DBG) $(DEP_STLDBG) | sed 's/^.*://;s/\\\\$$//'`

tags:	$(OUTPUT_DIRS) release-shared-dep dbg-shared-dep stldbg-shared-dep
	@ctags -d -g -i -m -t `cat -s $(DEP) $(DEP_DBG) $(DEP_STLDBG) | sed 's/^.*://;s/\\\\$$//'`

-include $(DEPENDS_COLLECTION)

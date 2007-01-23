# Time-stamp: <07/01/23 14:58:21 ptr>
#
# Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
# Petr Ovtchenkov
#
# Portion Copyright (c) 1999-2001
# Parallel Graphics Ltd.
#
# Licensed under the Academic Free License version 3.0
#

PHONY += release-static-dep release-shared-dep dbg-static-dep dbg-shared-dep \
         depend

ifndef WITHOUT_STLPORT
PHONY += stldbg-static-dep stldbg-shared-dep
endif

release-static-dep release-shared-dep:	$(DEP)

dbg-static-dep dbg-shared-dep:	$(DEP_DBG)

ifndef WITHOUT_STLPORT
stldbg-static-dep stldbg-shared-dep:	$(DEP_STLDBG)

_ALL_DEP := $(DEP) $(DEP_DBG) $(DEP_STLDBG)
_DASH_DEP := release-shared-dep dbg-shared-dep stldbg-shared-dep
else
_ALL_DEP := $(DEP) $(DEP_DBG)
_DASH_DEP := release-shared-dep dbg-shared-dep
endif


depend::	$(OUTPUT_DIRS) ${_DASH_DEP}
	@cat -s $(_ALL_DEP) /dev/null > $(DEPENDS_COLLECTION)

TAGS:	$(OUTPUT_DIRS) ${_DASH_DEP}
	@etags -i -m `cat -s $(_ALL_DEP) /dev/null | sed 's/^.*://;s/\\\\$$//' | uniq -u`

tags:	$(OUTPUT_DIRS) ${_DASH_DEP}
	@ctags -d --globals --declarations -t -w `cat -s $(_ALL_DEP) /dev/null | sed 's/^.*://;s/\\\\$$//' | uniq -u`

-include $(DEPENDS_COLLECTION)

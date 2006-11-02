# Time-stamp: <06/11/01 22:55:23 ptr>
#
# Copyright (c) 2006
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

ifdef SUBDIRS
# Do the same target in all catalogs from SUBDIRS
define doinsubdirs
@for d in ${SUBDIRS}; do \
  ${MAKE} -C $$d $@; \
done
endef

else
# Dummy, do nothing
define doinsubdirs
endef

endif


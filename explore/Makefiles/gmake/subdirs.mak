# Time-stamp: <06/11/01 22:55:23 ptr>
#
# Copyright (c) 2006
# Petr Ovtchenkov
#
# Licensed under the Academic Free License version 3.0
#

# Do the same target in all catalogs as arg
define doinsubdirs
for d in $(1); do \
  ${MAKE} -C $$d $@ || exit -1; \
done
endef

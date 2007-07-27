SUBDIRS := subdir

define doinsubdirs
for d in $(1); do \
  ${MAKE} -C $$d $@; \
done
endef


all:
	${call doinsubdirs,${SUBDIRS} }
	echo "Next"

.PHONY:	all


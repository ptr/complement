
all:	release-shared debug-shared

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
CMD = debug
else
CMD = release
endif

release-shared:
	echo ${CMD}

debug-shared:
	echo ${CMD}

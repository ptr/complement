# -*- Makefile -*- Time-stamp: <03/03/21 12:01:22 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

# $(foreach ${LIBNAMES})

define P_TEMPLATE
$(1):	$$($(1)_SRC_CPP)
	echo "$$+ $$@ "
endef

$(foreach name,${LIBNAMES},$(eval $(call P_TEMPLATE,$(name))))

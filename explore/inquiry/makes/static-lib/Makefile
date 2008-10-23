# -*- Makefile -*- Time-stamp: <03/03/21 12:01:22 ptr>

SRCROOT := ../../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

PHONY += qq
# $(foreach ${LIBNAMES})
qq:	testx2

testx2:	DEFS += -Dtestx2

define P_TEMPLATE
$(1):	$$($(1)_SRC_CPP)
	echo "$$+ $$@ $${DEFS}"
endef

$(foreach name,${LIBNAMES},$(eval $(call P_TEMPLATE,$(name))))

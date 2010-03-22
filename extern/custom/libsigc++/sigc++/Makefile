# -*- makefile -*- Time-stamp: <10/03/22 14:37:56 ptr>

SRCROOT := ../../../../explore

EXTRA_PRE = generate
EXTRA_PRE_DBG = generate
EXTRA_PRE_STLDBG = generate

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

#LIBSIGCXX_DIR ?= ${HOME}/labs/libsigc++-2.2.4.2
#LIBSIGCXX_DIR ?= ${HOME}/labs/libsigc++2
#LIBSIGCXX_SRC = ${LIBSIGCXX_DIR}/sigc++

%.h:	${LIBSIGCXX_SRC}/macros/%.h.m4
	m4 -I ${LIBSIGCXX_SRC}/macros $< >$@

functors/%.h:	${LIBSIGCXX_SRC}/functors/macros/%.h.m4
	m4 -I ${LIBSIGCXX_SRC}/macros -I ${LIBSIGCXX_SRC}/functors/macros $< >$@

adaptors/%.h:	${LIBSIGCXX_SRC}/adaptors/macros/%.h.m4
	m4 -I ${LIBSIGCXX_SRC}/macros -I ${LIBSIGCXX_SRC}/adaptors/macros $< >$@


adaptors/lambda/%.h:	${LIBSIGCXX_SRC}/adaptors/lambda/macros/%.h.m4
	m4 -I ${LIBSIGCXX_SRC}/macros -I ${LIBSIGCXX_SRC}/adaptors/lambda/macros $< >$@

adaptors/lambda/lambda.cc:	${LIBSIGCXX_SRC}/adaptors/lambda/macros/lambda.cc.m4
	m4 -I ${LIBSIGCXX_SRC}/macros -I ${LIBSIGCXX_SRC}/adaptors/lambda/macros $< >$@


INCLUDES += -I.. -I${LIBSIGCXX_DIR}

generate:	dirs class_slot.h hide.h limit_reference.h method_slot.h \
                object_slot.h retype.h signal.h slot.h \
                functors/functor_trait.h functors/mem_fun.h \
                functors/ptr_fun.h functors/slot.h \
                adaptors/adaptor_trait.h \
                adaptors/bind.h \
                adaptors/bind_return.h \
                adaptors/compose.h \
                adaptors/deduce_result_type.h \
                adaptors/exception_catch.h \
                adaptors/hide.h \
                adaptors/retype.h \
                adaptors/retype_return.h \
                adaptors/lambda/base.h \
                adaptors/lambda/group.h \
                adaptors/lambda/lambda.cc \
                adaptors/lambda/operator.h \
                adaptors/lambda/select.h

dirs:
	[ ! -d functors ] && mkdir functors || exit 0; \
        [ ! -d adaptors ] && mkdir adaptors || exit 0; \
        [ ! -d adaptors/lambda ] && mkdir adaptors/lambda || exit 0

PHONY += generate dirs

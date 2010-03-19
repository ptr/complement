# -*- makefile -*- Time-stamp: <10/03/18 09:37:48 ptr>

LIBNAME = glibmm-2.6
MAJOR = 4
MINOR = 0
PATCH = 3
SRC_CPP = ${GLIBMM_SRC}/class.cc                        \
        ${GLIBMM_SRC}/containers.cc                   \
        ${GLIBMM_SRC}/debug.cc                        \
        ${GLIBMM_SRC}/dispatcher.cc                   \
        ${GLIBMM_SRC}/error.cc                        \
        ${GLIBMM_SRC}/exception.cc                    \
        ${GLIBMM_SRC}/exceptionhandler.cc             \
        ${GLIBMM_SRC}/init.cc                         \
        ${GLIBMM_SRC}/interface.cc                    \
        ${GLIBMM_SRC}/main.cc                         \
        ${GLIBMM_SRC}/miscutils.cc                    \
        ${GLIBMM_SRC}/object.cc                       \
        ${GLIBMM_SRC}/objectbase.cc                   \
        ${GLIBMM_SRC}/pattern.cc                      \
        ${GLIBMM_SRC}/property.cc                     \
        ${GLIBMM_SRC}/propertyproxy.cc                \
        ${GLIBMM_SRC}/propertyproxy_base.cc           \
        ${GLIBMM_SRC}/quark.cc                        \
        ${GLIBMM_SRC}/random.cc                       \
        ${GLIBMM_SRC}/sarray.cc                       \
        ${GLIBMM_SRC}/signalproxy.cc                  \
        ${GLIBMM_SRC}/signalproxy_connectionnode.cc   \
        ${GLIBMM_SRC}/streamiochannel.cc              \
        ${GLIBMM_SRC}/stringutils.cc                  \
        ${GLIBMM_SRC}/threadpool.cc                   \
        ${GLIBMM_SRC}/timer.cc                        \
        ${GLIBMM_SRC}/timeval.cc                      \
        ${GLIBMM_SRC}/ustring.cc                      \
        ${GLIBMM_SRC}/utility.cc                      \
        ${GLIBMM_SRC}/value.cc                        \
        ${GLIBMM_SRC}/value_custom.cc                 \
        ${GLIBMM_SRC}/wrap.cc

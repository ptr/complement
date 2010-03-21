# -*- makefile -*- Time-stamp: <10/03/21 23:01:26 ptr>

LIBNAME = libxml++
MAJOR = 2
MINOR = 14
PATCH = 0
SRC_CC = ${LIBXMLXX_SRC}/attribute.cc \
         ${LIBXMLXX_SRC}/dtd.cc \
         ${LIBXMLXX_SRC}/document.cc \
         ${LIBXMLXX_SRC}/noncopyable.cc \
         ${LIBXMLXX_SRC}/keepblanks.cc \
         ${LIBXMLXX_SRC}/schema.cc \
         ${LIBXMLXX_SRC}/exceptions/exception.cc \
         ${LIBXMLXX_SRC}/exceptions/parse_error.cc \
         ${LIBXMLXX_SRC}/exceptions/validity_error.cc \
         ${LIBXMLXX_SRC}/exceptions/internal_error.cc \
         ${LIBXMLXX_SRC}/io/istreamparserinputbuffer.cc \
         ${LIBXMLXX_SRC}/io/outputbuffer.cc \
         ${LIBXMLXX_SRC}/io/ostreamoutputbuffer.cc \
         ${LIBXMLXX_SRC}/io/parserinputbuffer.cc \
         ${LIBXMLXX_SRC}/nodes/cdatanode.cc \
         ${LIBXMLXX_SRC}/nodes/commentnode.cc \
         ${LIBXMLXX_SRC}/nodes/contentnode.cc \
         ${LIBXMLXX_SRC}/nodes/entityreference.cc \
         ${LIBXMLXX_SRC}/nodes/element.cc \
         ${LIBXMLXX_SRC}/nodes/node.cc \
         ${LIBXMLXX_SRC}/nodes/processinginstructionnode.cc \
         ${LIBXMLXX_SRC}/nodes/textnode.cc \
         ${LIBXMLXX_SRC}/parsers/parser.cc \
         ${LIBXMLXX_SRC}/parsers/saxparser.cc \
         ${LIBXMLXX_SRC}/parsers/domparser.cc \
         ${LIBXMLXX_SRC}/parsers/textreader.cc \
         ${LIBXMLXX_SRC}/validators/validator.cc \
         ${LIBXMLXX_SRC}/validators/dtdvalidator.cc \
         ${LIBXMLXX_SRC}/validators/schemavalidator.cc

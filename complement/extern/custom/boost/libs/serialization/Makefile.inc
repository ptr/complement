# -*- makefile -*- Time-stamp: <06/06/02 19:38:29 ptr>

BOOST_SR_SRC = ${BOOST_INCLUDE_DIR}/libs/serialization/src
#BOOST_SR_SRC = src
LIBNAME = boost_serialization
MAJOR = 1
MINOR = 33
PATCH = 1
SRC_CPP = ${BOOST_SR_SRC}/basic_archive.cpp \
          ${BOOST_SR_SRC}/basic_archive_impl.cpp \
          ${BOOST_SR_SRC}/basic_iarchive.cpp \
          ${BOOST_SR_SRC}/basic_iserializer.cpp \
          ${BOOST_SR_SRC}/basic_oarchive.cpp \
          ${BOOST_SR_SRC}/basic_oserializer.cpp \
          ${BOOST_SR_SRC}/basic_pointer_iserializer.cpp \
          ${BOOST_SR_SRC}/basic_pointer_oserializer.cpp \
          ${BOOST_SR_SRC}/basic_serializer_map.cpp \
          ${BOOST_SR_SRC}/basic_text_iprimitive.cpp \
          ${BOOST_SR_SRC}/basic_text_oprimitive.cpp \
          ${BOOST_SR_SRC}/basic_xml_archive.cpp \
          ${BOOST_SR_SRC}/binary_iarchive.cpp \
          ${BOOST_SR_SRC}/binary_oarchive.cpp \
          ${BOOST_SR_SRC}/codecvt_null.cpp \
          ${BOOST_SR_SRC}/extended_type_info.cpp \
          ${BOOST_SR_SRC}/extended_type_info_no_rtti.cpp \
          ${BOOST_SR_SRC}/extended_type_info_typeid.cpp \
          ${BOOST_SR_SRC}/polymorphic_iarchive.cpp \
          ${BOOST_SR_SRC}/polymorphic_oarchive.cpp \
          ${BOOST_SR_SRC}/text_iarchive.cpp \
          ${BOOST_SR_SRC}/text_oarchive.cpp \
          ${BOOST_SR_SRC}/void_cast.cpp \
          ${BOOST_SR_SRC}/xml_grammar.cpp \
          ${BOOST_SR_SRC}/xml_iarchive.cpp \
          ${BOOST_SR_SRC}/xml_oarchive.cpp

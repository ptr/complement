# -*- makefile -*- Time-stamp: <09/07/17 23:45:40 ptr>

BOOST_SR_SRC = ${BOOST_INCLUDE_DIR}/libs/serialization/src
#BOOST_SR_SRC = src
LIBNAME = boost_serialization
MAJOR = 1
MINOR = 42
PATCH = 0
SRC_CPP = ${BOOST_SR_SRC}/basic_archive.cpp \
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
          ${BOOST_SR_SRC}/extended_type_info.cpp \
          ${BOOST_SR_SRC}/extended_type_info_no_rtti.cpp \
          ${BOOST_SR_SRC}/extended_type_info_typeid.cpp \
          ${BOOST_SR_SRC}/polymorphic_iarchive.cpp \
          ${BOOST_SR_SRC}/polymorphic_oarchive.cpp \
          ${BOOST_SR_SRC}/stl_port \
          ${BOOST_SR_SRC}/text_iarchive.cpp \
          ${BOOST_SR_SRC}/text_oarchive.cpp \
          ${BOOST_SR_SRC}/void_cast.cpp \
          ${BOOST_SR_SRC}/xml_grammar.cpp \
          ${BOOST_SR_SRC}/xml_iarchive.cpp \
          ${BOOST_SR_SRC}/xml_oarchive.cpp

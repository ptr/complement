# -*- makefile -*- Time-stamp: <10/03/15 22:58:42 ptr>

BOOST_S_SRC = ${BOOST_INCLUDE_DIR}/libs/system/src
BOOST_LOG_SRC_DIR = ${BOOST_LOG_DIR}/libs/log/src
LIBNAME = boost_log
MAJOR = 1
MINOR = 42
PATCH = 0
SRC_CPP = ${BOOST_LOG_SRC_DIR}/attribute_set.cpp \
          ${BOOST_LOG_SRC_DIR}/attribute_values_view.cpp \
          ${BOOST_LOG_SRC_DIR}/slim_string.cpp \
          ${BOOST_LOG_SRC_DIR}/code_conversion.cpp \
          ${BOOST_LOG_SRC_DIR}/core.cpp \
          ${BOOST_LOG_SRC_DIR}/stream_provider.cpp \
          ${BOOST_LOG_SRC_DIR}/severity_level.cpp \
          ${BOOST_LOG_SRC_DIR}/global_logger_storage.cpp \
          ${BOOST_LOG_SRC_DIR}/named_scope.cpp \
          ${BOOST_LOG_SRC_DIR}/process_id.cpp \
          ${BOOST_LOG_SRC_DIR}/timer.cpp \
          ${BOOST_LOG_SRC_DIR}/exceptions.cpp \
          ${BOOST_LOG_SRC_DIR}/sink_frontends.cpp \
          ${BOOST_LOG_SRC_DIR}/text_ostream_backend.cpp \
          ${BOOST_LOG_SRC_DIR}/text_file_backend.cpp \
          ${BOOST_LOG_SRC_DIR}/syslog_backend.cpp \
          ${BOOST_LOG_SRC_DIR}/thread_specific.cpp \
          ${BOOST_LOG_SRC_DIR}/trivial.cpp

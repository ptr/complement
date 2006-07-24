# -*- makefile -*- Time-stamp: <03/03/31 15:11:21 ptr>

BOOST_FS_SRC = ${BOOST_INCLUDE_DIR}/libs/date_time/src
LIBNAME = boost_dt
MAJOR = 1
MINOR = 33
PATCH = 1
SRC_CPP = ${BOOST_FS_SRC}/gregorian/greg_month.cpp ${BOOST_FS_SRC}/gregorian/greg_weekday.cpp ${BOOST_FS_SRC}/gregorian/date_generators.cpp

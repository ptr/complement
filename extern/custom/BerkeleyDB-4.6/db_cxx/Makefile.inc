# -*- makefile -*- Time-stamp: <09/03/19 09:11:25 ptr>

LIBNAME = db_cxx
MAJOR = 4
MINOR = 6
PATCH = 21

SRC_CPP = \
        ${BERKELEYDB_SRC}/cxx/cxx_db.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_dbc.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_dbt.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_env.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_except.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_lock.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_logc.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_mpool.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_multi.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_seq.cpp \
        ${BERKELEYDB_SRC}/cxx/cxx_txn.cpp


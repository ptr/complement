# -*- makefile -*- Time-stamp: <09/07/20 15:17:59 ptr>

# BERKELEYDB_SRC = ../../BerkeleyDB
# LIBNAME = db_cxx
LIBNAME = db_cxx
MAJOR = 4
MINOR = 8
PATCH = 24

SRC_STL = \
        ${BERKELEYDB_SRC}/stl/dbstl_container.cpp \
        ${BERKELEYDB_SRC}/stl/dbstl_resource_manager.cpp

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
       ${BERKELEYDB_SRC}/cxx/cxx_txn.cpp \
       ${SRC_STL}


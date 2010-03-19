# -*- makefile -*- Time-stamp: <10/03/15 23:42:28 ptr>

SRCROOT := ../../../../../explore

# BOOST_LOG_DIR ?= ${HOME}/labs/boost-log/boost-log

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

BOOST_LOG_INCLUDE_DIR ?= ${BOOST_LOG_DIR}

INCLUDES += -I${BOOST_INCLUDE_DIR} -I${BOOST_LOG_INCLUDE_DIR}
DEFS += -DBOOST_LOG_SETUP_DLL -DBOOST_LOG_SETUP_BUILDING_THE_LIB=1

ifdef USE_SYSTEM_BOOST
release-shared:	LDLIBS = -lboost_date_time -lboost_filesystem -lboost_system -lboost_thread -lboost_regex -lboost_log
dbg-shared:	LDLIBS = -lboost_date_time -lboost_filesystem -lboost_system -lboost_thread -lboost_regex -lboost_log
else
release-shared:	LDLIBS = -L../date_time/${OUTPUT_DIR} -L../filesystem/${OUTPUT_DIR} -L../system/${OUTPUT_DIR} -L../thread/${OUTPUT_DIR} -L../log/${OUTPUT_DIR} -L../regex/${OUTPUT_DIR} -lboost_dt -lboost_fs -lboost_system -lboost_thread -lboost_regex -lboost_log
dbg-shared:	LDLIBS = -L../date_time/${OUTPUT_DIR_DBG} -L../filesystem/${OUTPUT_DIR_DBG} -L../system/${OUTPUT_DIR_DBG} -L../thread/${OUTPUT_DIR_DBG} -L../log/${OUTPUT_DIR_DBG} -L../regex/${OUTPUT_DIR_DBG} -lboost_dtg -lboost_fsg -lboost_systemg -lboost_threadg -lboost_regexg -lboost_logg
ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS = -L../date_time/${OUTPUT_DIR_STLDBG} -L../filesystem/${OUTPUT_DIR_STLDBG} -L../system/${OUTPUT_DIR_STLDBG} -L../thread/${OUTPUT_DIR_STLDBG} -L../log/${OUTPUT_DIR_STLDBG} -L../regex/${OUTPUT_DIR_STLDBG} -lboost_dtstlg -lboost_fsstlg -lboost_systemstlg -lboost_threadstlg -lboost_regexstlg -lboost_logstlg
endif
endif

check:	all
	${MAKE} -C test check

check-release-shared:	release-shared
	${MAKE} -C test check-release-shared

check-dbg-shared:	dbg-shared
	${MAKE} -C test check-dbg-shared

ifndef WITHOUT_STLPORT
check-stldbg-shared:	stldbg-shared
	${MAKE} -C test check-stldbg-shared
endif


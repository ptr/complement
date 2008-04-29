# -*- makefile -*- Time-stamp: <08/04/29 16:18:23 yeti>

SRCROOT := ../../../../explore
# POST_INSTALL = ln -sf ${SO_NAME} $(INSTALL_LIB_DIR)/libboost_date_time.so

FCGI_SRC = /export/hostel/labs/FastCGI/fcgi-2.4.0

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR} -I${FASTCGIDAEMON_SRC}/../include -I${FASTCGIDAEMON_SRC} -I../include -I/usr/include/libxml2 -I${FCGI_SRC}/include

release-shared:	LDLIBS += -L../../boost/libs/thread/${OUTPUT_DIR} -lboost_thread -L../../boost/libs/regex/${OUTPUT_DIR} -lboost_regex  -L../../fcgi/libfcgi/${OUTPUT_DIR} -lfcgi -L../../fcgi/libfcgi++/${OUTPUT_DIR} -lfcgi++ -L../library/${OUTPUT_DIR} -lfastcgi-daemon -ldl -lxml2

dbg-shared:	LDLIBS += -L../../boost/libs/thread/${OUTPUT_DIR_DBG} -lboost_threadg -L../../boost/libs/regex/${OUTPUT_DIR_DBG} -lboost_regexg  -L../../fcgi/libfcgi/${OUTPUT_DIR_DBG} -lfcgig -L../../fcgi/libfcgi++/${OUTPUT_DIR_DBG} -lfcgi++g -L../library/${OUTPUT_DIR_DBG} -lfastcgi-daemong -ldl -lxml2

ifndef WITHOUT_STLPORT
stldbg-shared:	LDLIBS += -L../../boost/libs/thread/${OUTPUT_DIR_STLDBG} -lboost_threadstlg -L../../boost/libs/regex/${OUTPUT_DIR_STLDBG} -lboost_regexstlg  -L../../fcgi/libfcgi/${OUTPUT_DIR_STLDBG} -lfcgistlg -L../../fcgi/libfcgi++/${OUTPUT_DIR_STLDBG} -lfcgi++stlg -L../library/${OUTPUT_DIR_STLDBG} -lfastcgi-daemonstlg -ldl -lxml2
endif

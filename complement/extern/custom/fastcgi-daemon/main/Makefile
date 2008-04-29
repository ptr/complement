# -*- makefile -*- Time-stamp: <08/04/29 12:25:34 yeti>

SRCROOT := ../../../../explore
# POST_INSTALL = ln -sf ${SO_NAME} $(INSTALL_LIB_DIR)/libboost_date_time.so

FCGI_SRC = /export/hostel/labs/FastCGI/fcgi-2.4.0

include Makefile.inc
include ${SRCROOT}/Makefiles/gmake/top.mak

INCLUDES += -I${BOOST_INCLUDE_DIR} -I${FASTCGIDAEMON_SRC}/../include -I${FASTCGIDAEMON_SRC} -I../include -I/usr/include/libxml2 -I${FCGI_SRC}/include

release-shared:	LDLIBS += -L../../boost/libs/thread/${OUTPUT_DIR} -lboost_thread -L../../boost/libs/regex/${OUTPUT_DIR} -lboost_regex  -L../../fcgi/libfcgi/${OUTPUT_DIR} -lfcgi -L../../fcgi/libfcgi++/${OUTPUT_DIR} -lfcgi++ -L../library/${OUTPUT_DIR} -lfastcgi-daemon -ldl -lxml2

# 
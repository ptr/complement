# -*- makefile -*- Time-stamp: <08/04/28 16:11:18 yeti>

LOG4CXX_SRC = /export/hostel/labs/FastCGI/log4cxx-0.9.7/src
LIBNAME = log4cxx
MAJOR = 0
MINOR = 9
PATCH = 7
SRC_CPP = ${LOG4CXX_SRC}/appenderattachableimpl.cpp \
          ${LOG4CXX_SRC}/appenderskeleton.cpp \
          ${LOG4CXX_SRC}/asyncappender.cpp \
          ${LOG4CXX_SRC}/basicconfigurator.cpp \
          ${LOG4CXX_SRC}/boundedfifo.cpp \
          ${LOG4CXX_SRC}/class.cpp \
          ${LOG4CXX_SRC}/condition.cpp \
          ${LOG4CXX_SRC}/configurator.cpp \
          ${LOG4CXX_SRC}/consoleappender.cpp \
          ${LOG4CXX_SRC}/criticalsection.cpp \
          ${LOG4CXX_SRC}/cyclicbuffer.cpp \
          ${LOG4CXX_SRC}/dailyrollingfileappender.cpp \
          ${LOG4CXX_SRC}/datagrampacket.cpp \
          ${LOG4CXX_SRC}/datagramsocket.cpp \
          ${LOG4CXX_SRC}/datelayout.cpp \
          ${LOG4CXX_SRC}/dateformat.cpp \
          ${LOG4CXX_SRC}/defaultcategoryfactory.cpp \
          ${LOG4CXX_SRC}/domconfigurator.cpp \
          ${LOG4CXX_SRC}/event.cpp \
          ${LOG4CXX_SRC}/fallbackerrorhandler.cpp \
          ${LOG4CXX_SRC}/fileappender.cpp \
          ${LOG4CXX_SRC}/filewatchdog.cpp \
          ${LOG4CXX_SRC}/formattinginfo.cpp \
          ${LOG4CXX_SRC}/gnomexml.cpp \
          ${LOG4CXX_SRC}/hierarchy.cpp \
          ${LOG4CXX_SRC}/htmllayout.cpp \
          ${LOG4CXX_SRC}/inetaddress.cpp \
          ${LOG4CXX_SRC}/layout.cpp\
          ${LOG4CXX_SRC}/level.cpp \
          ${LOG4CXX_SRC}/levelmatchfilter.cpp \
          ${LOG4CXX_SRC}/levelrangefilter.cpp \
          ${LOG4CXX_SRC}/loader.cpp\
          ${LOG4CXX_SRC}/locale.cpp\
          ${LOG4CXX_SRC}/logger.cpp \
          ${LOG4CXX_SRC}/loggingevent.cpp \
          ${LOG4CXX_SRC}/loglog.cpp \
          ${LOG4CXX_SRC}/logmanager.cpp \
          ${LOG4CXX_SRC}/msxml.cpp \
          ${LOG4CXX_SRC}/mutex.cpp \
          ${LOG4CXX_SRC}/ndc.cpp \
          ${LOG4CXX_SRC}/mdc.cpp \
          ${LOG4CXX_SRC}/nteventlogappender.cpp \
          ${LOG4CXX_SRC}/objectimpl.cpp \
          ${LOG4CXX_SRC}/odbcappender.cpp \
          ${LOG4CXX_SRC}/onlyonceerrorhandler.cpp \
          ${LOG4CXX_SRC}/optionconverter.cpp \
          ${LOG4CXX_SRC}/patternconverter.cpp \
          ${LOG4CXX_SRC}/patternlayout.cpp \
          ${LOG4CXX_SRC}/patternparser.cpp \
          ${LOG4CXX_SRC}/properties.cpp \
          ${LOG4CXX_SRC}/propertyconfigurator.cpp \
          ${LOG4CXX_SRC}/propertyresourcebundle.cpp \
          ${LOG4CXX_SRC}/propertysetter.cpp \
          ${LOG4CXX_SRC}/resourcebundle.cpp \
          ${LOG4CXX_SRC}/rollingfileappender.cpp \
          ${LOG4CXX_SRC}/rootcategory.cpp \
          ${LOG4CXX_SRC}/serversocket.cpp \
          ${LOG4CXX_SRC}/semaphore.cpp \
          ${LOG4CXX_SRC}/simplelayout.cpp \
          ${LOG4CXX_SRC}/smtpappender.cpp \
          ${LOG4CXX_SRC}/socket.cpp \
          ${LOG4CXX_SRC}/socketappender.cpp \
          ${LOG4CXX_SRC}/sockethubappender.cpp \
          ${LOG4CXX_SRC}/socketimpl.cpp \
          ${LOG4CXX_SRC}/socketinputstream.cpp \
          ${LOG4CXX_SRC}/socketnode.cpp \
          ${LOG4CXX_SRC}/socketoutputstream.cpp \
          ${LOG4CXX_SRC}/stringhelper.cpp \
          ${LOG4CXX_SRC}/stringmatchfilter.cpp \
          ${LOG4CXX_SRC}/stringtokenizer.cpp \
          ${LOG4CXX_SRC}/syslogappender.cpp \
          ${LOG4CXX_SRC}/syslogwriter.cpp \
          ${LOG4CXX_SRC}/system.cpp \
          ${LOG4CXX_SRC}/telnetappender.cpp \
          ${LOG4CXX_SRC}/timezone.cpp \
          ${LOG4CXX_SRC}/transform.cpp \
          ${LOG4CXX_SRC}/thread.cpp \
          ${LOG4CXX_SRC}/threadspecificdata.cpp \
          ${LOG4CXX_SRC}/ttcclayout.cpp \
          ${LOG4CXX_SRC}/writerappender.cpp \
          ${LOG4CXX_SRC}/xmllayout.cpp\
          ${LOG4CXX_SRC}/xmlsocketappender.cpp

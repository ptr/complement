# Time-stamp: <00/05/16 13:17:30 ptr>
# $SunId$

# Run as
# NMAKE /NOLOGO /F VC.mak /R DBG=Release
#   or
# NMAKE /NOLOGO /F VC.mak /R DBG=Debug

LIBNAME=EDS
CFG=LIB

all:	lib

PRG_OBJ = $(TAGETDIR)/_EventHandler.obj \
          $(TAGETDIR)/NetTransport.obj \
          $(TAGETDIR)/EvManager.obj \
          $(TAGETDIR)/EvPack.obj \
          $(TAGETDIR)/crc.obj \
          $(TAGETDIR)/_SessionMgr.obj \
          $(TAGETDIR)/Names.obj \
          $(TAGETDIR)/Cron.obj


!INCLUDE ../../lib.mak

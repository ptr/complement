# Time-stamp: <00/02/23 14:19:26 ptr>
# $SunId$ %Q%

LIBNAME=EDS

PRG_OBJ = $(TAGETDIR)/_EventHandler.obj \
          $(TAGETDIR)/NetTransport.obj \
          $(TAGETDIR)/EvManager.obj \
          $(TAGETDIR)/EvPack.obj \
          $(TAGETDIR)/crc.obj \
          $(TAGETDIR)/_SessionMgr.obj \
          $(TAGETDIR)/Names.obj \
          $(TAGETDIR)/Cron.obj


!INCLUDE ../../lib.mak

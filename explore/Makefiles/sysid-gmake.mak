# Time-stamp: <03/07/06 19:58:37 ptr>
# $Id$

ifndef BUILD_DATE

OSNAME := $(shell uname -s | tr '[A-Z]' '[a-z]' | tr ', /\\()"' ',//////' | tr ',/' ',-')
OSREL := $(shell uname -r | tr '[A-Z]' '[a-z]' | tr ', /\\()"' ',//////' | tr ',/' ',-')
M_ARCH := $(shell uname -m | tr '[A-Z]' '[a-z]' | tr ', /\\()"' ',//////' | tr ',/' ',-')
P_ARCH := $(shell uname -p | tr '[A-Z]' '[a-z]' | tr ', /\\()"' ',//////' | tr ',/' ',-')
NODENAME := $(shell uname -n | tr '[A-Z]' '[a-z]' )
SYSVER := $(shell uname -v )
USER := $(shell echo $$USER )

# OS_VER := $(shell uname -s | tr '[A-Z]' '[a-z]' | tr ', /\\()"' ',//////' | tr ',/' ',_')

BUILD_SYSTEM := $(shell echo `uname -n` `uname -s` `uname -r` `uname -v` `uname -m` $$USER)
BUILD_DATE := $(shell date +'%Y/%m/%d %T %Z')

endif

.SUFFIXES:
.SCCS_GET:

#!/bin/sh

# $Id$

BASEDIR=`dirname \`dirname \\\`dirname ${PWD}\\\`\``
LD_LIBRARY_PATH=${BASEDIR}/build/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH
PRG=sig

o=/tmp/test.$$

obj/gcc/shared-g/${PRG} > $o 2>&1

if diff ./output $o ; then
  echo "Ok"
else
  echo "Fail"
fi

rm -f $o

obj/gcc/shared-stlg/${PRG} > $o 2>&1

if diff ./output $o ; then
  echo "Ok"
else
  echo "Fail"
fi

rm -f $o

obj/gcc/shared/${PRG} > $o 2>&1

if diff ./output $o ; then
  echo "Ok"
else
  echo "Fail"
fi

rm -f $o

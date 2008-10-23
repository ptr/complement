#!/bin/sh

# $Id$

BASEDIR=`dirname \`dirname \\\`dirname \\\\\\\`dirname ${PWD}\\\\\\\` \\\`\``
LD_LIBRARY_PATH=${BASEDIR}/build/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH
PRG=func
echo ${BASEDIR}
o=/tmp/test.$$

psql -f ./Create.pg.sql template1 > /dev/null 2>&1


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

psql -f ./Drop.pg.sql template1 > /dev/null 2>&1

exit 0

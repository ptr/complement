#!/bin/sh

d=`dirname \`dirname \\\`pwd\\\`\``
time=`dirname $d`/app/utils/time/obj/gcc/shared/time
# time="/usr/bin/time -f \"%U %S %e\""
d=`dirname $d`/build/lib

LD_LIBRARY_PATH=$d:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH

echo=echo

w_lnum () {
  if [ -f $1 ] ; then
    ${echo} `wc -l < $1` "\c" >> $1
  else
    ${echo} "0 \c" > $1
  fi
}

experiment () {
  w_lnum server-$3.log
  # start server
  ${time} -a -o server-$3.log obj/gcc/shared/serv -p=1990 -b=$1 &
  srv=$!
  # start client
  w_lnum client-$3.log
  sleep 2
  ${time} -a -o client-$3.log ../Client-04/obj/gcc/shared/client -p=1990 -host=localhost -n=$2 -b=$1
  wait $srv
  sleep 1
}

repeat () {
  nn=$1
  i=0
  shift
  while [ $i -lt $nn ] ; do
    $*
    i=`expr $i + 1`
  done
  ${echo} "-----------"
  sleep 1
}

ls=32
ns=1638400

l0=1024
n0=51200

lp=4096
np=12800

lb=1048576
nb=50

lh=52428800
nh=1

# repeat 10 experiment $ls $ns s
# repeat 10 experiment $l0 $n0 0
# repeat 10 experiment $lp $np p
repeat 10 experiment $lb $nb b
repeat 10 experiment $lh $nh h

${echo} "# Server $ls/$ns"
${echo} "$ls \c"
../Server-03/stat.awk server-s.log
${echo} "# Client $ls/$ns"
${echo} -e "$ls \c"
../Server-03/stat.awk client-s.log

${echo} "# Server $l0/$n0"
${echo} "$l0 \c"
../Server-03/stat.awk server-0.log
${echo} "# Client $l0/$n0"
${echo} "$l0 \c"
../Server-03/stat.awk client-0.log

${echo} "# Server $lp/$np"
${echo} "$lp \c"
../Server-03/stat.awk server-p.log
${echo} "# Client $lp/$np"
${echo} "$lp \c"
../Server-03/stat.awk client-p.log

${echo} "# Server $lb/$nb"
${echo} "$lb \c"
../Server-03/stat.awk server-b.log
${echo} "# Client $lb/$nb"
${echo} "$lb \c"
../Server-03/stat.awk client-b.log

${echo} "# Server $lh/$nh"
${echo} "$lh \c"
../Server-03/stat.awk server-h.log
${echo} "# Client $lh/$nh"
${echo} "$lh \c"
../Server-03/stat.awk client-h.log

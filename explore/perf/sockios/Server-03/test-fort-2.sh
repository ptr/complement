#!/bin/sh

d=`dirname \`dirname \\\`pwd\\\`\``
time=`dirname $d`/app/utils/time/obj/gcc/shared/time
time_rm=/mnt/fort/ptr/workshop/explore/app/utils/time/obj/gcc/shared/time
# time="/usr/bin/time -f \"%U %S %e\""
d=`dirname $d`/build/lib

LD_LIBRARY_PATH=$d:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH


w_lnum () {
  if [ -f $1 ] ; then
    echo -e `wc -l < $1` "\c" >> $1
  else
    echo -e "0 \c" > $1
  fi
}

experiment () {
  w_lnum server-fort-$3.log
  # start server
  ${time} -a -o server-fort-$3.log obj/gcc/shared/serv -p=1990 -b=$1 &
  srv=$!
  # start client
  w_lnum client-oldnavy-$3.log
  ssh oldnavy "( cd /mnt/fort/ptr/workshop/explore/perf/sockios/Server-03; setenv LD_LIBRARY_PATH /mnt/fort/ptr/workshop/explore/build/lib:/home/ptr/lib:/usr/local/lib:/usr/lib; ${time_rm} -a -o client-oldnavy-$3.log ../Client-03/obj/gcc/shared/client -p=1990 -host=fort -n=$2 -b=$1 )"
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
  echo "-----------"
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


repeat 10 experiment $ls $ns s
repeat 10 experiment $l0 $n0 0
repeat 10 experiment $lp $np p
repeat 10 experiment $lb $nb b
repeat 10 experiment $lh $nh h

echo "# Server $ls/$ns"
echo -e "$ls \c"
./stat.awk server-fort-s.log
echo "# Client $ls/$ns"
echo -e "$ls \c"
./stat.awk client-oldnavy-s.log

echo "# Server $l0/$n0"
echo -e "$l0 \c"
./stat.awk server-fort-0.log
echo "# Client $l0/$n0"
echo -e "$l0 \c"
./stat.awk client-oldnavy-0.log

echo "# Server $lp/$np"
echo -e "$lp \c"
./stat.awk server-fort-p.log
echo "# Client $lp/$np"
echo -e "$lp \c"
./stat.awk client-oldnavy-p.log

echo "# Server $lb/$nb"
echo -e "$lb \c"
./stat.awk server-fort-b.log
echo "# Client $lb/$nb"
echo -e "$lb \c"
./stat.awk client-oldnavy-b.log

echo "# Server $lh/$nh"
echo -e "$lh \c"
./stat.awk server-fort-h.log
echo "# Client $lh/$nh"
echo -e "$lh \c"
./stat.awk client-oldnavy-h.log

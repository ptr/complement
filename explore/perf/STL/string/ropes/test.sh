#!/bin/sh

d=`dirname \`dirname \\\`pwd\\\`\``
d=`dirname $d`
#d=`dirname $d`
time=`dirname $d`/app/utils/time/obj/gcc/shared/time
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
  #w_lnum test$3.log
  ${time} -a -o test$3.log STLport-default/obj/gcc/shared/str $3 -i=$2 -b=$1
  echo -e ".\c"
}

repeat () {
  nn=$1
  i=0
  shift
  while [ $i -lt $nn ] ; do
    $*
    i=`expr $i + 1`
  done
  echo ""
  echo "-----------"
}

block=64
iter=100000

run () {
  rm -f test-s.log
  repeat 10 experiment $1 $2 -s
  echo "# strings: block $1 iterations $2"
  echo $1 `./stat.awk test-s.log` >> string.dat
  echo $1 `./stat.awk test-s.log`
  rm -f test-r.log
  repeat 10 experiment $1 $2 -r
  echo "# ropes: block $1 iterations $2"
  echo $1 `./stat.awk test-r.log` >> rope.dat
  echo $1 `./stat.awk test-r.log`
}

rm -f string.dat rope.dat

run $block $iter
run 128 100000
run 512 100000
run 1024 100000
run 4096 100000
run 10240 100000
run 20480 100000
run 30420 100000
run 40960 100000
run 51200 100000
run 81920 100000
run 102400 100000
## run 1024000 10000

exit 0

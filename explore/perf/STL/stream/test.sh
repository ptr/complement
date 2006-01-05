#!/bin/sh
#
# Time-stamp: <06/01/04 00:33:20 ptr>
#

BASEDIR=${PWD}/../../..
timeprg=${BASEDIR}/app/utils/time/obj/gcc/shared/time
# time='/usr/bin/time'

function runtest ()
{
  rm -f s.log
  i=0
  while [ $i -lt 10 ] ; do
    echo -n .
    let i=i+1
    $timeprg -a -o s.log "$1/obj/gcc/shared/str"
  done
  echo
  echo =========
}

rm -f libstdc++.dat
touch libstdc++.dat

j=1
for d in fstream-format fstream-raw sstream-raw ; do
  runtest ${d}/libstdc++
  echo $j `../stat.awk s.log` >> libstdc++.dat
  let j=j+1
done

rm -f STLport.dat
touch STLport.dat

j=1
for d in fstream-format fstream-raw sstream-raw ; do
  runtest ${d}/STLport-default
  echo $j `../stat.awk s.log` >> STLport.dat
  let j=j+1
done

rm -f STLport-malloc.dat
touch STLport-malloc.dat

j=1
for d in fstream-format fstream-raw sstream-raw ; do
  runtest ${d}/STLport-malloc
  echo $j `../stat.awk s.log` >> STLport-malloc.dat
  let j=j+1
done

rm -f stdio.dat
touch stdio.dat

j=1
for d in fstream-format fstream-raw sstream-raw ; do
  runtest ${d}/stdio
  echo $j `../stat.awk s.log` >> stdio.dat
  let j=j+1
done

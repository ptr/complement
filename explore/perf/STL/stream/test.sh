#!/bin/sh
#
# Time-stamp: <05/12/31 00:18:31 ptr>
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

rm -f libstd++.dat
touch libstd++.dat

j=1
for d in fstream-format fstream-raw sstream-raw ; do
  runtest ${d}/libstd++
  echo $j `../stat.awk s.log` >> libstd++.dat
  let j=j+1
done
# dummy:
echo $j `../stat.awk s.log` >> libstd++.dat

rm -f STLport.dat
touch STLport.dat

j=1
for d in fstream-format fstream-raw sstream-raw ; do
  runtest ${d}/STLport-default
  echo $j `../stat.awk s.log` >> STLport.dat
  let j=j+1
done
runtest params-short/STLport-no-short-str
echo $j `../stat.awk s.log` >> STLport.dat

rm -f STLport-malloc.dat
touch STLport-malloc.dat

j=1
for d in fstream-format fstream-raw sstream-raw ; do
  runtest ${d}/STLport-malloc
  echo $j `../stat.awk s.log` >> STLport-malloc.dat
  let j=j+1
done



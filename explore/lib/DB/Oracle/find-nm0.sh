#!/bin/sh

ORACLE_HOME=/oracle/app/oracle/product/8.1.6

for name in `find . \( -name "*.a" -o -name "*.o" \) -print`; do
  res=`nm $name | grep $1`
  if [ ! "$res" = "" ] ; then
    echo "============> $name";
    echo $res;
  fi
done

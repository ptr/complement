#!/bin/sh

libpath=`dirname $0`
export LD_PRELOAD=${libpath}/obj/gcc/shared/libtest.so.0.0

exec $1

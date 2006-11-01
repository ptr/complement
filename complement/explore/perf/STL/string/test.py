#!/usr/bin/env python
#
# Time-stamp: <04/07/15 01:16:23 ptr>
#
# $Id$
#

import sys
import os
import time

BASEDIR = os.path.normpath( os.getcwd() + "/../../../" );
timeprg = BASEDIR + "/app/utils/time/obj/gcc/shared/time"
# time = '/usr/bin/time'

#os.environ["LD_LIBRARY_PATH"] = BASEDIR + '/build/lib:' + os.environ["LD_LIBRARY_PATH"]

def test( d ):
  os.unlink( 's.log' )
  for i in range(10):
    print '.',
    os.spawnve( os.P_WAIT, timeprg, ['', '-a', '-o', 's.log', d + '/obj/gcc/shared/str' ], os.environ )

test( 'add/libstd++' )
os.system( 'echo 1 `../stat.awk s.log` > libstd++.dat' )
time.sleep(60*3)
test( 'find/libstd++' )
os.system( 'echo 2 `../stat.awk s.log` >> libstd++.dat' )
time.sleep(60*3)
test( 'ops/libstd++' )
os.system( 'echo 3 `../stat.awk s.log` >> libstd++.dat' )
time.sleep(60*3)
test( 'params/libstd++' )
os.system( 'echo 4 `../stat.awk s.log` >> libstd++.dat' )
time.sleep(60*3)
test( 'params-ref/libstd++' )
os.system( 'echo 5 `../stat.awk s.log` >> libstd++.dat' )
time.sleep(60*5)
test( 'params-short/libstd++' )
os.system( 'echo 6 `../stat.awk s.log` >> libstd++.dat' )
os.system( 'echo 7 `../stat.awk s.log` >> libstd++.dat' )

time.sleep(60*8)

test( 'add/STLport-default' )
os.system( 'echo 1 `../stat.awk s.log` > STLport.dat' )
time.sleep(60*3)
test( 'find/STLport-default' )
os.system( 'echo 2 `../stat.awk s.log` >> STLport.dat' )
time.sleep(60*3)
test( 'ops/STLport-default' )
os.system( 'echo 3 `../stat.awk s.log` >> STLport.dat' )
time.sleep(60*3)
test( 'params/STLport-default' )
os.system( 'echo 4 `../stat.awk s.log` >> STLport.dat' )
time.sleep(60*3)
test( 'params-ref/STLport-default' )
os.system( 'echo 5 `../stat.awk s.log` >> STLport.dat' )
time.sleep(60*3)
test( 'params-short/STLport-default' )
os.system( 'echo 6 `../stat.awk s.log` >> STLport.dat' )
time.sleep(60*3)
test( 'params-short/STLport-no-short-str' )
os.system( 'echo 7 `../stat.awk s.log` >> STLport.dat' )

time.sleep(60*8)

test( 'add/STLport-malloc' )
os.system( 'echo 1 `../stat.awk s.log` > STLport-malloc.dat' )
time.sleep(60*3)
test( 'find/STLport-malloc' )
os.system( 'echo 2 `../stat.awk s.log` >> STLport-malloc.dat' )
time.sleep(60*3)
test( 'ops/STLport-malloc' )
os.system( 'echo 3 `../stat.awk s.log` >> STLport-malloc.dat' )
time.sleep(60*3)
test( 'params/STLport-malloc' )
os.system( 'echo 4 `../stat.awk s.log` >> STLport-malloc.dat' )
time.sleep(60*3)
test( 'params-ref/STLport-malloc' )
os.system( 'echo 5 `../stat.awk s.log` >> STLport-malloc.dat' )

time.sleep(60*8)

test( 'add/STLport-newalloc' )
os.system( 'echo 1 `../stat.awk s.log` > STLport-newalloc.dat' )
time.sleep(60*3)
test( 'find/STLport-newalloc' )
os.system( 'echo 2 `../stat.awk s.log` >> STLport-newalloc.dat' )
time.sleep(60*3)
test( 'ops/STLport-newalloc' )
os.system( 'echo 3 `../stat.awk s.log` >> STLport-newalloc.dat' )
time.sleep(60*3)
test( 'params/STLport-newalloc' )
os.system( 'echo 4 `../stat.awk s.log` >> STLport-newalloc.dat' )
time.sleep(60*3)
test( 'params-ref/STLport-newalloc' )
os.system( 'echo 5 `../stat.awk s.log` >> STLport-newalloc.dat' )

time.sleep(60*8)

test( 'add-ropes/STLport-default' )
os.system( 'echo 1 `../stat.awk s.log` > STLport-ropes.dat' )
time.sleep(60*3)
test( 'find-ropes/STLport-default' )
os.system( 'echo 2 `../stat.awk s.log` >> STLport-ropes.dat' )
time.sleep(60*3)
test( 'ops-ropes/STLport-default' )
os.system( 'echo 3 `../stat.awk s.log` >> STLport-ropes.dat' )
time.sleep(60*3)
test( 'params-ropes/STLport-default' )
os.system( 'echo 4 `../stat.awk s.log` >> STLport-ropes.dat' )
time.sleep(60*3)
test( 'params-ref-ropes/STLport-default' )
os.system( 'echo 5 `../stat.awk s.log` >> STLport-ropes.dat' )


#!/usr/bin/env python
#
# Time-stamp: <03/07/23 23:40:42 ptr>
#
# $Id$
#

import sys
import os

BASEDIR = os.path.normpath( os.getcwd() + "/../../../" );
time = BASEDIR + "/app/utils/time/obj/gcc/shared/time"
# time = '/usr/bin/time'

os.environ["LD_LIBRARY_PATH"] = BASEDIR + '/build/lib:' + os.environ["LD_LIBRARY_PATH"]

def test( d ):
  os.unlink( 's.log' )
  for i in range(10):
    print '.',
    os.spawnve( os.P_WAIT, time, ['', '-a', '-o', 's.log', d + '/obj/gcc/shared/str' ], os.environ )

os.system( 'cd add; make -f Makefile-g++-3 clobber depend all' )
test( 'add' )
os.system( 'echo 1 `../stat.awk s.log` > g++-3.dat' )

os.system( 'cd find; make -f Makefile-g++-3 clobber depend all' )
test( 'find' )
os.system( 'echo 2 `../stat.awk s.log` >> g++-3.dat' )

os.system( 'cd ops; make -f Makefile-g++-3 clobber depend all' )
test( 'ops' )
os.system( 'echo 3 `../stat.awk s.log` >> g++-3.dat' )

os.system( 'cd params; make -f Makefile-g++-3 clobber depend all' )
test( 'params' )
os.system( 'echo 4 `../stat.awk s.log` >> g++-3.dat' )

os.system( 'cd params-ref; make -f Makefile-g++-3 clobber depend all' )
test( 'params-ref' )
os.system( 'echo 5 `../stat.awk s.log` >> g++-3.dat' )

os.system( 'cd add; make -f Makefile-STLport clobber depend all' )
test( 'add' )
os.system( 'echo 1 `../stat.awk s.log` > STLport.dat' )

os.system( 'cd find; make -f Makefile-STLport clobber depend all' )
test( 'find' )
os.system( 'echo 2 `../stat.awk s.log` >> STLport.dat' )

os.system( 'cd ops; make -f Makefile-STLport clobber depend all' )
test( 'ops' )
os.system( 'echo 3 `../stat.awk s.log` >> STLport.dat' )

os.system( 'cd params; make -f Makefile-STLport clobber depend all' )
test( 'params' )
os.system( 'echo 4 `../stat.awk s.log` >> STLport.dat' )

os.system( 'cd params-ref; make -f Makefile-STLport clobber depend all' )
test( 'params-ref' )
os.system( 'echo 5 `../stat.awk s.log` >> STLport.dat' )

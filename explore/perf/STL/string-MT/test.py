#!/usr/bin/env python
#
# Time-stamp: <03/07/23 23:41:51 ptr>
#
# $Id$
#

import sys
import os

BASEDIR = os.path.normpath( os.getcwd() + "/../../../" );
time = BASEDIR + "/app/utils/time/obj/gcc/shared/time"
# time = '/usr/bin/time'

en = os.environ["LD_LIBRARY_PATH"]

def test( d, lib ):
  os.unlink( 's.log' )
  os.environ["LD_LIBRARY_PATH"] = lib + ':' + en
  for i in range(10):
    print '.',
    os.spawnve( os.P_WAIT, time, ['', '-a', '-o', 's.log', d + '/obj/gcc/shared/str' ], os.environ )

os.system( 'cd add; make -f Makefile-g++-3 clobber depend all' )
test( 'add', '.' )
os.system( 'echo 1 `../stat.awk s.log` > g++-3.dat' )

os.system( 'cd find; make -f Makefile-g++-3 clobber depend all' )
test( 'find', '.' )
os.system( 'echo 2 `../stat.awk s.log` >> g++-3.dat' )

os.system( 'cd ops; make -f Makefile-g++-3 clobber depend all' )
test( 'ops', '.' )
os.system( 'echo 3 `../stat.awk s.log` >> g++-3.dat' )

os.system( 'cd params; make -f Makefile-g++-3 clobber depend all' )
test( 'params', '.' )
os.system( 'echo 4 `../stat.awk s.log` >> g++-3.dat' )

os.system( 'cd params-ref; make -f Makefile-g++-3 clobber depend all' )
test( 'params-ref', '.' )
os.system( 'echo 5 `../stat.awk s.log` >> g++-3.dat' )

os.system( 'cd add; make -f Makefile-STLport clobber depend all' )
test( 'add', '/export/home/ptr/STLport.lab/STLport/lib' )
os.system( 'echo 1 `../stat.awk s.log` > STLport.dat' )

os.system( 'cd find; make -f Makefile-STLport clobber depend all' )
test( 'find', '/export/home/ptr/STLport.lab/STLport/lib' )
os.system( 'echo 2 `../stat.awk s.log` >> STLport.dat' )

os.system( 'cd ops; make -f Makefile-STLport clobber depend all' )
test( 'ops', '/export/home/ptr/STLport.lab/STLport/lib' )
os.system( 'echo 3 `../stat.awk s.log` >> STLport.dat' )

os.system( 'cd params; make -f Makefile-STLport clobber depend all' )
test( 'params', '/export/home/ptr/STLport.lab/STLport/lib' )
os.system( 'echo 4 `../stat.awk s.log` >> STLport.dat' )

os.system( 'cd params-ref; make -f Makefile-STLport clobber depend all' )
test( 'params-ref', '/export/home/ptr/STLport.lab/STLport/lib' )
os.system( 'echo 5 `../stat.awk s.log` >> STLport.dat' )

os.system( 'cd add; make -f Makefile-STLport.malloc clobber depend all' )
test( 'add', '/export/home/ptr/STLport.lab/STLport.malloc/lib' )
os.system( 'echo 1 `../stat.awk s.log` > STLport-malloc.dat' )

os.system( 'cd find; make -f Makefile-STLport.malloc clobber depend all' )
test( 'find', '/export/home/ptr/STLport.lab/STLport.malloc/lib' )
os.system( 'echo 2 `../stat.awk s.log` >> STLport-malloc.dat' )

os.system( 'cd ops; make -f Makefile-STLport.malloc clobber depend all' )
test( 'ops', '/export/home/ptr/STLport.lab/STLport.malloc/lib' )
os.system( 'echo 3 `../stat.awk s.log` >> STLport-malloc.dat' )

os.system( 'cd params; make -f Makefile-STLport.malloc clobber depend all' )
test( 'params', '/export/home/ptr/STLport.lab/STLport.malloc/lib' )
os.system( 'echo 4 `../stat.awk s.log` >> STLport-malloc.dat' )

os.system( 'cd params-ref; make -f Makefile-STLport.malloc clobber depend all' )
test( 'params-ref', '/export/home/ptr/STLport.lab/STLport.malloc/lib' )
os.system( 'echo 5 `../stat.awk s.log` >> STLport-malloc.dat' )
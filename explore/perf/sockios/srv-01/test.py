#!/opt/python-2.2.1/bin/python
#
# Time-stamp: <02/09/10 14:54:19 ptr>
#
# $Id$
#

import sys
import os

BASEDIR = "/export/home/ptr/workshop/explore"
SRV_PID = "./srv.pid"
PORT = 1990
NUM = 10000

os.environ["LD_LIBRARY_PATH"] = BASEDIR + '/build/lib:' + os.environ["LD_LIBRARY_PATH"]

pid = os.fork()
if ( pid == 0 ):
  # print os.getpid()
  os.execve( "/usr/bin/time", \
             ['', '-f', '%E %S %U %P %r %s', '--quiet', \
              'srv-01/obj/gcc/shared/srv', \
              '-pid=' + SRV_PID, \
              "-p=" + str(PORT) ], \
             os.environ )
  # os.execve( 'srv-01/obj/gcc/shared/srv', [''], os.environ )
  # os.system( 'srv-01/obj/gcc/shared/srv' )
  # os.execve( "/bin/echo", ['1', '2', '3' ], os.environ )
  # print os.getpid()
  # sys.exit( 0 )

no_srv = 1;
i = 0
while ( no_srv ): # wait for server begin listen socket
  try:
    pf = open( SRV_PID )
    srvpid = int(pf.readline())
    pf.close()
    no_srv = 0

  except IOError:
    ++i
    if ( i > 100 ):
      no_srv = 0
      os.kill( pid, 9 ) # INT
      print "Can't start server, exit"
      os.exit( 0 )


print 'Server started'
print 'Client: '
os.system( "/usr/bin/time -f '%E %S %U %P %r %s' cln-01/obj/gcc/shared/cln -p=" + str(PORT) + ' -host=localhost -n=' + str(NUM) )
print 'Client done'
print 'Server:'
os.kill( srvpid, 2 ) # INT
# print 'My pid', os.getpid()
os.waitpid( pid, 0 )
print 'Server done'
os.unlink( SRV_PID )

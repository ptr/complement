#!/usr/bin/env python
#
# Time-stamp: <03/09/01 16:18:12 ptr>
#
# $Id: smtp-ut-aux.py,v 1.1 2003/09/11 10:18:11 ptr Exp $
#

import unittest
import os
import string
# import re
import socket
import sys
import time

smtpgw_host = 'localhost'
smtpgw_port = 10025
helo_name = 'peak.avp.ru'
mail_from = 'ptr@peak.avp.ru'
recpt_to = 'smtpgw-test@peak.avp.ru'

os.unlink( 'session.log' )

class RAWclient:
  def __init__(self, host, port ):
    self.s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.s.connect( (host, port) )
    self.fs = os.fdopen( self.s.fileno(), 'r+b' )
    self.sess = open( "session.log", "ab" )
    print >> self.sess, "=+", time.strftime( '%Y-%m-%d %H:%M:%S %Z' )


  def close(self):
    self.fs.close()
    self.s.close()
    print >> self.sess, '=-', time.strftime( '%Y-%m-%d %H:%M:%S %Z\n' )
    self.sess.close()

  def command( self, str ):
    self.fs.write( str + '\r\n' )
    self.sess.write( '<- ' + str + '\r\n' )

  def response(self):
    txt = []
    line = self.fs.readline()
    self.sess.write( '-> ' + line )
    while ((len(line) > 3) and (line[3] == '-')):
      txt.append( line[4:] )
      line = self.fs.readline()
      self.sess.write( '-> ' + line )
    
    txt.append( line[4:] )
    return (int(string.split(line)[0]), txt)

  def rawline(self):
    line = self.fs.readline()
    self.sess.write( '-> ' + line )
    return line
  
  def write( self, str ):
    self.fs.write( str )
    str = string.replace( str, '\n', '\n<- ' )
    self.sess.write( '<- ' + str + '\n' )

class TestAux(unittest.TestCase):
  
  def setUp(self):
    self.rc = RAWclient( smtpgw_host, smtpgw_port )
    
    self.greeting = self.rc.rawline()
    
    self.rc.command( "EHLO " + helo_name )
    code, lines = self.rc.response()
    
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
  def tearDown(self):
    self.rc.command( "QUIT" )
    code, lines = self.rc.response()
    self.rc.close()
    del self.rc
    
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 2 )

  def testNOOP(self):
    self.rc.command( "NOOP" )
    code, lines = self.rc.response()
    # print code, lines
    self.failUnless( code != 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
  def testUnrecognize(self):
    self.rc.command( "SOME command" )
    code, lines = self.rc.response()
    # print code, lines
    self.failUnless( code == 500 )

  def testVerify(self):
    self.rc.command( "VRFY " + recpt_to )
    code, lines = self.rc.response()
    # print code, lines
    self.failUnless( code == 252 )
    
  def testExpand(self):
    self.rc.command( "EXPN " + recpt_to )
    code, lines = self.rc.response()
    # print code, lines
    self.failUnless( code != 500 )
    
  def testHelp(self):
    self.rc.command( "HELP" )
    code, lines = self.rc.response()
    # print code, lines
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 1 )
    
  def testResetSession(self):
    self.rc.command( "MAIL FROM:<" + mail_from + '>' )
    code, lines = self.rc.response()
    # print code, lines
    self.failUnless( code != 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
    self.rc.command( "RCPT TO:<" + recpt_to + '>' )
    code, lines = self.rc.response()
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
    self.rc.command( "DATA" )
    code, lines = self.rc.response()
    # print code, lines
    self.failUnless( code == 354 )
    self.rc.write( "Subject: SMTP GW test fail\n\n" )
    self.rc.write( "This should not happen!\n" )
    self.rc.write( "\r\n.\r\n" )
    code, lines = self.rc.response()
    # print code, lines

    self.rc.command( "RSET" )
    code, lines = self.rc.response()
    # print code, lines

#if __name__ == '__main__':
#  unittest.main()

ts = unittest.TestSuite()
ts.addTest( unittest.makeSuite(TestAux) )
result = unittest.TextTestRunner( verbosity=2 ).run( ts )

if not result.wasSuccessful():
  f = open( 'session.log' )
  print f.read()
  f.close()
  sys.exit(-1)

#!/usr/bin/env python
#
# Time-stamp: <03/08/27 11:56:46 ptr>
#
# $Id: smtp-ut.py,v 1.1 2003/09/11 10:18:11 ptr Exp $
#

import unittest
import os
import string
# import re
import socket
import sys

smtpgw_host = 'localhost'
smtpgw_port = 10025
helo_name = 'peak.avp.ru'
mail_from = 'ptr@peak.avp.ru'
recpt_to = 'ptr@peak.avp.ru'

def getcode( sock ):
  txt = []
  line = sock.readline()
  while ((len(line) > 3) and (line[3] == '-')):
    txt.append( line[4:] )
    line = sock.readline()
  txt.append( line[4:] )
  return (int(string.split(line)[0]), txt)

class SimpleTest(unittest.TestCase):
  
  def setUp(self):
    self.s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.s.connect( (smtpgw_host, smtpgw_port) )
    self.fs = os.fdopen( self.s.fileno(), 'r+b' )
    # self.greeting = self.s.recv( 4096 )
    self.greeting = self.fs.readline()
    
  def tearDown(self):
    self.s.close()

  def testGreeting(self):
    code = int(string.split( self.greeting )[0])
    # print self.greeting
    self.failUnless( code == 220 )
    self.failUnless( code < 400 )

  def testHELO(self):
    # self.s.sendall( "HELO peak.avp.ru\r\n" )
    self.fs.write( "HELO " + helo_name + '\r\n' )
    # answer = self.s.recv( 4096 )
    code, lines = getcode( self.fs )
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )

  def testQUIT(self):
    # self.s.sendall( "QUIT\r\n" )
    # answer = self.s.recv( 4096 )
    self.fs.write( "QUIT\r\n" )
    code, lines = getcode( self.fs )
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 2 )
    
class SimpleTest2(unittest.TestCase):
  
  def setUp(self):
    self.s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.s.connect( (smtpgw_host, smtpgw_port) )
    self.fs = os.fdopen( self.s.fileno(), 'r+b' )
    self.greeting = self.fs.readline()
    
  def tearDown(self):
    self.s.close()

  def testGreeting(self):
    code = int(string.split( self.greeting )[0])
    # print self.greeting
    self.failUnless( code == 220 )
    self.failUnless( code < 400 )

  def testHELO(self):
    self.fs.write( "EHLO " + helo_name + '\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )

  def testQUIT(self):
    self.fs.write( "QUIT\r\n" )
    code, lines = getcode( self.fs )
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 2 )

class TestBrokenConnection(unittest.TestCase):
  
  def setUp(self):
    self.s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.s.connect( (smtpgw_host, smtpgw_port) )
    self.fs = os.fdopen( self.s.fileno(), 'r+b' )
    self.greeting = self.fs.readline()
    self.fs.write( "EHLO " + helo_name + '\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
  def tearDown(self):
    # self.fs.write( "QUIT\r\n" )
    # code, lines = getcode( self.fs )
    # self.failUnless( code < 400 )
    # self.failUnless( (code // 100) == 2 )
    # self.failUnless( ((code % 100) // 10) == 2 )
    # self.s.close()
    pass

  def testGreeting(self):
    code = int(string.split( self.greeting )[0])
    # print self.greeting
    self.failUnless( code == 220 )
    self.failUnless( code < 400 )

  def testSend0(self):
    self.fs.write( "MAIL FROM:<" + mail_from + '>\r\n')
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
    self.fs.write( "RCPT TO:<" + recpt_to + '>\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
    self.fs.write( "DATA\r\n" )
    code, lines = getcode( self.fs )
    # print code, lines
    # Illegal! Server fault here!
    self.s.close()

class SimpleData1(unittest.TestCase):
  
  def setUp(self):
    self.s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.s.connect( (smtpgw_host, smtpgw_port) )
    self.fs = os.fdopen( self.s.fileno(), 'r+b' )
    self.greeting = self.fs.readline()
    self.fs.write( "EHLO " + helo_name + '\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
  def tearDown(self):
    self.fs.write( "QUIT\r\n" )
    code, lines = getcode( self.fs )
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 2 )
    self.s.close()

  def testGreeting(self):
    code = int(string.split( self.greeting )[0])
    # print self.greeting
    self.failUnless( code == 220 )
    self.failUnless( code < 400 )

  def testSend0(self):
    self.fs.write( "MAIL FROM:<" + mail_from + '>\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code != 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
    self.fs.write( "RCPT TO:<" + recpt_to + '>\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
    self.fs.write( "DATA\r\n" )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code == 354 )
    self.fs.write( "Subject: SMTP GW test\n\n" )
    self.fs.write( "test\n" )
    self.fs.write( "\r\n.\r\n" )
    code, lines = getcode( self.fs )
    # print code, lines

class SimpleData2(unittest.TestCase):
  
  def setUp(self):
    self.s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.s.connect( (smtpgw_host, smtpgw_port) )
    self.fs = os.fdopen( self.s.fileno(), 'r+b' )
    self.greeting = self.fs.readline()
    self.fs.write( "EHLO " + helo_name + '\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
  def tearDown(self):
    self.fs.write( "QUIT\r\n" )
    code, lines = getcode( self.fs )
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 2 )
    self.s.close()

  def testGreeting(self):
    code = int(string.split( self.greeting )[0])
    # print self.greeting
    self.failUnless( code == 220 )
    self.failUnless( code < 400 )

  def testSend0(self):
    self.fs.write( "MAIL FROM:<" + mail_from + '>\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code != 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
    self.fs.write( "RCPT TO:<" + recpt_to + '>\r\n' )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code < 400 )
    self.failUnless( (code // 100) == 2 )
    self.failUnless( ((code % 100) // 10) == 5 )
    
    self.fs.write( "DATA\r\n" )
    code, lines = getcode( self.fs )
    # print code, lines
    self.failUnless( code == 354 )
    self.fs.write( "test more\n" )
    self.fs.write( "\r\n.\r\n" )
    code, lines = getcode( self.fs )
    # print code, lines

    
#if __name__ == '__main__':
#  unittest.main()

ts = unittest.TestSuite()
ts.addTest( unittest.makeSuite(SimpleTest) )
ts.addTest( unittest.makeSuite(SimpleTest2) )
ts.addTest( unittest.makeSuite(TestBrokenConnection) )
ts.addTest( unittest.makeSuite(SimpleData1) )
ts.addTest( unittest.makeSuite(SimpleData2) )
result = unittest.TextTestRunner( verbosity=2 ).run( ts )

if not result.wasSuccessful():
  sys.exit(-1)

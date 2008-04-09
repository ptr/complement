#!/usr/bin/env python
#
# Time-stamp: <04/01/28 14:05:37 ptr>
#
# $Id: smtp-batch.py,v 1.6 2004/06/16 14:21:17 ptr Exp $
#

# import unittest
import os
import string
import socket
import timeoutsocket
import sys
import time
import re

import mimetypes
from email import Encoders
from email.Message import Message
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.MIMEMessage import MIMEMessage
from email.MIMEMultipart import MIMEMultipart

# os.unlink( 'session.log' )

#REwords = re.compile("((?:\".*?\")|(?:(?:\S(?:\\ )*)+))")
#REwords = re.compile( "\s+")
REwords = re.compile( "(?:\'.*?\')|(?:\".*?\")|(?:(?:\S|(?:\\\s))+)")
REnl = re.compile( "^\r\n" )
REqoutes = re.compile( "(?:\'.*?\')|(?:\".*?\")" )
# REcomment = re.compile( "[^\"]*?#" )

timeoutsocket.setDefaultSocketTimeout( 180 )

ret_code = 0

class RAWclient:
  def __init__(self, host, port ):
    # self.s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.s = timeoutsocket.socket( timeoutsocket.AF_INET, timeoutsocket.SOCK_STREAM )
    self.s.connect( (host, port) )
    self.fs = os.fdopen( self.s.fileno(), 'r+b' )
    self.sess = open( "session.log", "ab" )
    print >> self.sess, "==", sys.argv[1], sys.argv[2:]
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

  def write_silent( self, str ):
    self.fs.write( str )
    
  def write( self, str ):
    self.fs.write( str )
    if REnl.match( str ) != None:
      self.sess.write( '<- \n' )
      str = REnl.sub( '', str )
    self.sess.write( '<- ' + str )

  def mark_in_log( self, str ):
    print >> self.sess, '!!', str
    
  def import_in_log( self, str ):
    print >> self.sess, '<=', str


def subst_args( line, argv ):
  try:
    p = 0
    mp = 0
    while p >= 0:
      m = REqoutes.search( line, mp )
      p = string.find( line, '$', p ) # .... replace
      if m is not None and p > m.start() and p < m.end():
        p = m.end()
        mp = m.end() + 1
        continue
      if m is not None and p > m.end():
        mp = m.end() + 1
      if p >= 0:
        i = int( line[p+1] ) - 1
        if i > len(argv):
          ret_code = -2
          raise "wrong arg reference"
        line = line[0:p] + argv[i] + line[p+2:]
        p += len(argv[i])
  except ValueError:
    print line
    print p, m
    sys.exit(1)
  return line


if len(sys.argv) == 1:
  print "Usage:"
  print sys.argv[0], "<test case filename>"
  sys.exit(1)
else:
  script_name = sys.argv[1]
  script_args = sys.argv[2:]

commands = []

script = open( script_name )

verbatim_mode = 0
verbatim_code = ''
last_command = ''

line = script.readline()
while line:
  if verbatim_mode == 1:
    if string.find( line, '<<<' + verbatim_code ) == 0 \
           and string.strip( line ) == '<<<' + verbatim_code:
      verbatim_mode = 0
      verbatim_code = ''
    
    commands.append( [line] )
    line = script.readline()
    continue
  if verbatim_mode == 0 and string.find( line, '>>>' ) == 0:
    verbatim_mode = 1
    verbatim_code = string.strip( line[3:] )
    commands.append( [line] )
    line = script.readline()
    continue
  line = string.strip( line )
  # Remove comments:
  p = 0
  mp = 0
  while p >= 0:
    p = string.find( line, '#', p )
    m = REqoutes.search( line, mp )
    if m is not None and p > m.start() and p < m.end():
      # print p, m.start, m.end
      p = m.end()
      mp = p + 1;
      continue
    if p >= 0:
      line = string.strip( line[0:p] )
      break
    if m is not None and p > m.end():
      mp = m.end() + 1
  if len( line ) > 0:
    # Substitute arguments:
    line = subst_args( line, script_args )
    # Split line, store list of tokens
    # commands.append( string.split(line) )
    # print "'" + line + "'"
    # print string.split(line)
    # print REwords.split(line)
    l = []
    for r in REwords.findall( line ):
      # strip quotation
      if r[0] == '"' or r[0] == '\'':
        r = r[1:-1]
      l.append( r )
    # print l
    commands.append( l )
    
  line = script.readline()

script.close()

global client

count = 0
send_flag = 0

# Ok, now let's interpret commands:
for c in commands:
  if verbatim_mode == 1:
    # we in the 'inline include' (verbatim)
    # check end marker of verbatim mode:
    if string.find( c[0], '<<<' + verbatim_code ) == 0 \
           and string.strip( c[0] ) == '<<<' + verbatim_code:
      verbatim_mode = 0
      verbatim_code = ''
      if last_command == 'DATA':
        # send end-of-mail marker (\r\n.\r\n)
        client.write( '\r\n.\r\n' )
        last_command = ''
    else:
      client.write( c[0] )
    count += 1
    continue
  if verbatim_mode == 0 and string.find( c[0], '>>>' ) == 0:
    # enter in verbatim mode
    verbatim_mode = 1
    verbatim_code = string.strip( c[0][3:] )
    count += 1
    # send header X-smtpgw-test (in data, after command 'send', see below)
    # This header is used to track mail propagation
    if send_flag == 1:
      client.write( 'X-smtpgw-test: ' + id + ' action=recv' + '\n' )
      send_flag = 0
    continue
  if c[0] == 'connect':
    # print 'connect ->', c[1], c[2]
    client = RAWclient( c[1], int(c[2]) )
  elif c[0] in ('HELO', 'EHLO', 'VRFY', 'EXPN'):
    # print c[0], c[1]
    last_command = c[0]
    client.command( c[0] + ' ' + c[1] )
  elif c[0] in ( 'NOOP', 'RSET', 'QUIT', 'DATA' ):
    # print c[0]
    last_command = c[0]
    client.command( c[0] )
  elif c[0] == 'HELP': # "c[0] in ( 'HELP' )" is buggy for Python 2.1, 2.2.2
    last_command = c[0]
    if len(c) == 1:
      client.command( c[0] )
    else:
      client.command( c[0] + ' ' + c[1] )
  elif c[0] in ('MAIL', 'RCPT'):
    last_command = c[0]
    if len(c) != 2:
      print 'Bad syntax on line', count + 1, "\n", c
    client.command( c[0] + ' ' + c[1] )
  elif c[0] == 'disconnect':
    # print c[0]
    client.close()
    # del client
  elif c[0] == 'expected':
    code, msg = client.response()
    if len(c) > 1:
      if ( int(c[1]) != code ):
        print "Unexpected return code after command", last_command + ":", code, msg
        print "Line:", count + 1
        client.mark_in_log( "Unexpected return code" )
        ret_code = -1
  elif c[0] == 'send':
    # notify mail propagation tracker that we sent mail
    t = time.time()
    id = 'id=' + str( t ) + '-' + socket.gethostname()
    #scoll = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    scoll = timeoutsocket.socket( timeoutsocket.AF_INET, timeoutsocket.SOCK_STREAM )
    # scoll.connect( ('localhost', 2049) )
    # print "connect to", c[1], int(c[2])
    scoll.connect( (c[1], int(c[2])) )
    # fscoll = os.fdopen( scoll.fileno(), 'r+b' )
    fscoll = timeoutsocket.TimeoutFile( scoll )
    # s.makefile( "r+b" )
    fscoll.write( id + ' ' + 'action=out\n' )
    fscoll.flush()
    send_flag = 1
    #scoll.close()
  elif c[0] == 'wait':
    # wait responce from mail propagation tracker, that one receive our mail
    # (try-block work with timeoutsocket python module)
    try:
      resp = fscoll.readline()
      print resp, "time elapsed: ", time.time() - t
      fscoll.close()
      scoll.close()
    except timeoutsocket.Timeout:
      print "fail, out of time;", "time elapsed: ", time.time() - t
      fscoll.close()
      scoll.close()     
  elif c[0] == 'include':
    # include verbatim file
    if last_command == 'DATA' and send_flag == 1:
      # send header X-smtpgw-test (in data, after command 'send', see above)
      # This header is used to track mail propagation
      client.write( 'X-smtpgw-test: ' + id + ' action=recv' + '\n' )
      send_flag = 0

    # don't write whole file in log, but write what file we sent
    client.import_in_log( 'import "' + c[1] + '"')
    # nice, expected that in *.eml file all already done
    # send it as is
    if c[1][-4:] == '.eml':
      datafile = open( c[1] )
      dline = datafile.readline()
      while dline:
        client.write_silent( dline )
        dline = datafile.readline()
      
      if last_command == 'DATA':
        client.write( '\r\n.\r\n' )
        last_command = ''
      
      datafile.close()
    else:
      # otherwise, build MIME multipart message
      ctype, encoding = mimetypes.guess_type( c[1] )
      if ctype is None or encoding is not None:
        ctype = 'application/octet-stream'
      maintype, subtype = ctype.split( '/', 1 )
      # msg = MIMEMultipart()
      genmsg = MIMEMultipart()
      msg = MIMEBase( maintype, subtype )
      genmsg['Subject'] = 'smtpgw test suite, multipart'
      genmsg['From'] = 'some@peak.avp.ru'
      genmsg['To'] = 'other@peak.avp.ru'
      genmsg.preamble = 'smtpgw test'
      genmsg.epilogue = ''
      datafile = open( c[1] )
      msg.set_payload( datafile.read() )
      datafile.close()
      Encoders.encode_base64(msg)
      msg.add_header( 'Content-Disposition', 'attachment', filename=c[1] )
      genmsg.attach( msg )
      client.write_silent( genmsg.as_string() )
      if last_command == 'DATA':
        client.write( '\r\n.\r\n' )
        last_command = ''
      
  else:
    print 'Unrecognized command:', c
    print 'Line: ', count + 1 # not true: comments and blank lines was removed
    ret_code = -3
  count += 1

if ret_code != 0:
  sys.exit( ret_code )

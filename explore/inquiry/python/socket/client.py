#!/usr/bin/env python
#
# Time-stamp: <03/08/20 13:25:35 ptr>
#
# $Id$
#

#import sys
#import os
#import string
#import re

#from stat import *

import socket

s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
s.connect( ('localhost', 2000) )

s.send( "Hi!\n" )

s.close()

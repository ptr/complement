#!/usr/bin/env perl

use ExtUtils::testlib;
use echoclient;

echoclient::send_message( "Hello", 5 );
# echoclient::send_message( "Hello", 5 );
echoclient::wait_stem();


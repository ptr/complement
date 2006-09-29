#!/usr/bin/env perl

use ExtUtils::testlib;

use stem::NetTransport;

my $conn = new stem::NetTransport;
my $test_obj = new stem::mprocessor( "mytest" );

my $ns_addr = $conn->open( "localhost", 6995 );

#print $ns_addr, "\n";
$test_obj->send( $ns_addr, "string" );


#$conn->close();
$conn->join();

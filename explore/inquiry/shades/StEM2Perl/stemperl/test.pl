#!/usr/bin/env perl

use ExtUtils::testlib;

use stem;

my $conn = new stem::NetTransport;
my $test_obj = new stem::mprocessor( "mytest" );

my $ns_addr = $conn->open( "localhost", 6995 );

# print $ns_addr, "\n";
$test_obj->send( $ns_addr, "string" );
# $test_obj->send( $test_obj->self_id(), "string" );

#sleep( 3 );

print $test_obj->get_with_timeout( 0, 500000000 ), "\n" or die;

#print $test_obj->get() or die;


$conn->close();
$conn->join();

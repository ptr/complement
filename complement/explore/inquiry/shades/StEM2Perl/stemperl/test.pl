#!/usr/bin/env perl

use ExtUtils::testlib;

use stem;

my $conn = new stem::NetTransport;
my $test_obj = new stem::mprocessor( "mytest" );

my $ns_addr = $conn->open( "localhost", 6995 );

if ( !$conn->good() ) {
  die "Peer not available\n";
}

# print $ns_addr, "\n";
$test_obj->send( $ns_addr, "string" );
# $test_obj->send( $test_obj->self_id(), "string" );

#sleep( 3 );

print $test_obj->get_with_timeout( 0, 500000000 ), "\n" or die;

# my %ns = $conn->names();
my @a = $conn->names();

#while (($key,$val) = each %ns) {
#  print $key, ' = ',  $val, "\n";
#}

for ( my $i = 0; $i < $#a; ++$i ) {
  print $a[$i], " => ", $a[++$i], "\n";
}


#print $test_obj->get() or die;


$conn->close();
$conn->join();

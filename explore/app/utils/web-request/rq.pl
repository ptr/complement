#!/usr/local/bin/perl

use URI::URL;
use LWP::UserAgent;
use HTTP::Request;
use HTTP::Request::Common;
use HTTP::Request::Form;
use HTML::TreeBuilder 3.0;

my $ua = LWP::UserAgent->new( # env_proxy => 1,
                             keep_alive => 1,
                             timeout => 30, );
my $url = url 'http://bayesweb.wadsworth.org/cgi-bin/gibbs.7.pl?data_type=DNA';
# $ua->proxy( [http], 'http://xeon.paragraph.ru:8080/' );
$ua->agent( 'Mozilla/5.0 (X11; U; SunOS i86pc; en-US; rv:1.0rc1) Gecko/20020423' );
my $res = $ua->request(GET $url);

my $tree = HTML::TreeBuilder->new;
$tree->parse($res->content);
$tree->eof();

# begin of loop for read some files in directory and send ones contents

my @forms = $tree->find_by_tag_name('FORM');
die "What, no forms in $url?" unless @forms;
my $f = HTTP::Request::Form->new($forms[0], $url);
$f->field( "motif_lengths", "22" );
# instead of '>32....' should be file content...
$f->field( "data", '>32A5UTR Human alpha-Bcrystallin gene, 5\' end
GTCGACACCACCCAAAATAGTGCCGAGCCTCTTGGGGGGGGAGGGGCTGGGAGTGGGGG
CCCTGAGTGAGAGCAACGAGGGTGTGACCAGCGCCGCCCGGACCCCTAGTCCCCTCCCC
CGCACACTCTTCAGCTGTCGCAGGGGGCCTGAGAGGACAGCTGAGGGTCCTGGCTGGGA
ACGAGCTGGGGAGGGGGAGCTGGTGGTGCCTGGGGCATGAAGAGGCCTCGCTGAGACCC
TCACAAACGGTTTGCACGTTTCCACACCTCATTTTCTCCTCTTCGGTGGCAGGCACTGT
GCACCCAATTCCTAAAGCACTCCTGGATTTAATGTTCTGAGAGCCACATAGAACGAAAG
ATGCAAGAAATCTGTTTGCTCTTTTTTCAGGGGGTGGGGTCTTTCTGCCCAGATGTGGG
ATCCTCTCCTAAACCCAGGTCAACCCAGGGCACGAGGCAGATGGCTGGTGCTGACATGT
TGACCATCACTGCTCTCTTCCAAGGACTCACAAAGAGTTAATGTCCCTGGGGCTCAGCC
TAGGAAGATTCCAGTCCCTGCCCAGGCCCAAGATAGTTGCTGGCCTGATTCCCCTGGCA
TTCAGGACTGGAAAGGAGGAGGAGGGGCACACTACGCCGGCTCCCATCCTCCCCCCACC
CCGCGTGCCTGCTTGGGATTCCTGACTCTGTACCAGCTTCAGAGAACAGGGGTGGGGGT
GGGTGCCATTGGGTGTGGACAGAAAGCTAGTGAAACAAGACCATGACAAGTCACTGGCC
GGCTCAGACGTGTTTGTGTCTCTCTTTTCTTAGCTCAGTGAGTACTGGGTATGTGTCAC
ATTGCCAAATCCCGGATCACAAGTCTCCATGAACTGCTGGTGAGCTAGGATAATAAAAC
CCCTGACATCACCATTCCAGAAGCTTCACAAGACTGCATATATAAGGGGCTGGCTGTAG
CTGCAGCTGAAGGAGCTGACCAGCCAGCT
');

my $response = $ua->request($f->press("submit"));
# print $response->content if $response->is_success;
print $response->content; # you may wish to write content instead of print one as here

# end of files send/receive loop


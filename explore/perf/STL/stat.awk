#! /usr/bin/nawk -f

BEGIN {
# if ( ARGC == 3 ) {
#   av = ARGV[2];
#   nn = ARGV[3];
#   y = 1;
# } else {
#   y = 0;
# }
  i1 = 0.0; i2 = 0.0; i3 = 0.0; n = 0;
  sq1 = 0; sq2 = 0; sq3 = 0; 
}
{
  i1 += $1;
  i2 += $2;
  i3 += $3;
  ++n;
  sq1 += $1 * $1;
  sq2 += $2 * $2;
  sq3 += $3 * $3;
}

END { print (i1 / n), sqrt((sq1 - (i1 * i1)/n ) / n), \
            (i2 / n), sqrt((sq2 - (i2 * i2)/n ) / n), \
            (i3 / n), sqrt((sq3 - (i3 * i3)/n ) / n); }

#!/bin/sh

tmpf1=/tmp/stat$$-1

awk '{ print $1;}' $1 > $tmpf1

exec 9>&0 <$tmpf1
i=1
while read x1; do
  cat >> scr <<EOF
# ----- begin
connect \$1 \$2
expected 220
EHLO \$3
expected 250
MAIL FROM:<\$4>
expected 250
RCPT TO:<\$5>
expected 250
send \$6 \$7
DATA
expected 354
include /opt/mcollection2/x2/$x1
expected 250
wait
QUIT
expected 221
disconnect
# ----- end
EOF
done
exec 0>&9 9>&-
rm -f $tmpf1

# This is sed script to generate MTA test script from output of command like
#   find /opt/mcollection/1/Disinfected/ -type f -print

i \
# ----- begin\
connect $1 $2\
expected 220\
EHLO $3\
expected 250\
MAIL FROM:<$4>\
expected 250\
RCPT TO:<$5>\
expected 250\
send $6 $7\
DATA\
expected 354
s/\(.*\)/include "\1"/
a \
expected 250\
wait\
QUIT\
expected 221\
disconnect\
# ----- end

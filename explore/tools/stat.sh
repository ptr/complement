#!/bin/sh

tmpf1=/tmp/stat$$-1
tmpf2=/tmp/stat$$-2
tmpf3=/tmp/stat$$-3
tmpf4=/tmp/stat$$-4
tmpf5=/tmp/stat$$-5

ls -lFSr . | awk 'BEGIN { i = 0; } { print $9, $5, i; i += 1;}' > $tmpf1

rm -f $tmpf2
exec 9>&0 <$tmpf1
i=1
while read x1 x2 x3; do
  case "$x2" in 
    "")
      ;;
    *)
      grep "Content-Type: multipart/mixed" $x1 > /dev/null
      if [ "$?" != '0' ]; then
        echo $x1 $x2 $x3 $i >> $tmpf2;
        let ++i
      else
        echo $x1 $x2 $x3 $i >> $tmpf2;
      fi
      ;;
  esac
done
exec 0>&9 9>&-
rm -f $tmpf1
rm -f $tmpf3

exec 9>&0 <$tmpf2
i=1
while read x1 x2 x3 x4; do
  case "$x2" in 
    "")
      ;;
    *)
      grep "Content-Type: text/plain" $x1 > /dev/null
      if [ "$?" != '0' ]; then
        echo $x1 $x2 $x3 $x4 $i >> $tmpf3;
        let ++i
      else
        echo $x1 $x2 $x3 $x4 $i >> $tmpf3;
      fi
      ;;
  esac
done
exec 0>&9 9>&-
rm -f $tmpf2

rm -f $tmpf4
exec 9>&0 <$tmpf3
i=1
while read x1 x2 x3 x4 x5; do
  case "$x2" in 
    "")
      ;;
    *)
      grep "Content-Type: text/html" $x1 > /dev/null
      if [ "$?" != '0' ]; then
        echo $x1 $x2 $x3 $x4 $x5 $i >> $tmpf4;
        let ++i
      else
        echo $x1 $x2 $x3 $x4 $x5 $i >> $tmpf4;
      fi
      ;;
  esac
done
exec 0>&9 9>&-
rm -f $tmpf3

rm -f $tmpf5
exec 9>&0 <$tmpf4
i=1
while read x1 x2 x3 x4 x5 x6; do
  case "$x2" in 
    "")
      ;;
    *)
      grep "Content-Disposition: attachment" $x1 > /dev/null
      if [ "$?" != '0' ]; then
        echo $x1 $x2 $x3 $x4 $x5 $x6 $i >> $tmpf5;
        let ++i
      else
        echo $x1 $x2 $x3 $x4 $x5 $x6 $i >> $tmpf5;
      fi
      ;;
  esac
done
exec 0>&9 9>&-
rm -f $tmpf4

cat $tmpf5
rm -f $tmpf5

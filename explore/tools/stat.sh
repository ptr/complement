#!/bin/sh

tmpf=/tmp/stat$$

ls -lFSr . | awk 'BEGIN { i = 0; } { print $9, $5, i; i += 1;}' > $tmpf

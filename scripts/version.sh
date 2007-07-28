#!/bin/sh

rev=$(LC_ALL=C svn info | grep "^Revision:" | cut -c 11-)
test -n "$rev" && echo "dev-r${rev}"


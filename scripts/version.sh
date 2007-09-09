#!/bin/sh

rev=$(svnversion . /svn/neverball/trunk | tr : +)
test -n "$rev" && test "$rev" != "exported" && echo "dev-r${rev}"


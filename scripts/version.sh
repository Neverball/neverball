#!/bin/sh

rev=$(svnversion . /svn/neverball/trunk)
test -n "$rev" && test "$rev" != "exported" && echo "dev-r${rev}"


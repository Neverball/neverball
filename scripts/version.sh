#!/bin/sh

LC_ALL=C
export LC_ALL

svn_version() {
    svn_rev="$(svnversion . /svn/neverball/trunk | tr : +)"
    test "$svn_rev" != "exported" && echo "$svn_rev"
}

svk_version() {
    svk info 2> /dev/null | grep -E "^Mirrored From:" |
    sed -e "s/^[^,]*, Rev\. //"
}

rev="$(svn_version || svk_version)"
test -n "$rev" && echo "dev-r$rev" || echo "unknown"


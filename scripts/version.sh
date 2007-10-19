#!/bin/sh

LC_ALL=C
export LC_ALL

svn_version() {
    svn_rev="$(svnversion . /svn/neverball/trunk | tr : +)"
    test "$svn_rev" != "exported" && echo "r$svn_rev"
}

svk_version() {
    if ! svk info &> /dev/null; then
        return
    fi

    svk_rev="$(svk info | grep -E "^Mirrored From:" | cut -d, -f2 | cut -c7-)"
    test -n "$svk_rev" && echo "r$svk_rev" || echo "svk"
}

version="$(svn_version || svk_version)"
test -n "$version" && echo "dev-$version" || echo "unknown"


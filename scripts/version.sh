#!/bin/sh

BUILD="$1"
VERSION="$2"
TEMPLATE_FILE="$3"
HEADER_FILE="$4"
CACHE_FILE="$5"

LC_ALL=C
export LC_ALL

svn_version()
{
    svn_rev="$(svnversion . /svn/neverball/trunk | tr : +)"
    [ "$svn_rev" != "exported" ] && echo "r$svn_rev"
}

if [ "$BUILD" != "release" ]; then
    VERSION="$(svn_version || date -u +"%Y-%m-%d" || echo "$VERSION-dev")"
fi

if [ "$VERSION" != "$(cat "$CACHE_FILE" 2> /dev/null)" ]; then
    sed 's,0\.0\.0,'"$VERSION"',' < "$TEMPLATE_FILE" > "$HEADER_FILE"
    echo "$VERSION" > "$CACHE_FILE" 2> /dev/null
fi

echo "$VERSION"

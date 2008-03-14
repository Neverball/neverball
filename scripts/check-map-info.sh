#!/bin/sh

LC_ALL=C
export LC_ALL

# Directories with Neverball levels
path="data/map-easy data/map-fwp data/map-hard \
data/map-medium data/map-mym data/map-mym2"

# Mandatory level meta-data
keys="message song shot goal time time_hs goal_hs coin_hs"

for key in $keys; do
    echo "Missing or empty \"$key\""
    echo

    {
        # Missing...
        find $path -name '*.map' |
        xargs egrep -L '^"'$key'"' |
        sed 's/^/    /'

        # ...empty.
        find $path -name '*.map' |
        xargs egrep -l '^"'$key'"[[:space:]]+"[[:space:]]*"$' |
        sed 's/^/    /'
    } | sort

    echo
done


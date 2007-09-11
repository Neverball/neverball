#!/bin/sh
# This script extracts translatable messages from maps and sets.
# Copyright (C) 2006 Jean Privat
#
# Part of the Neverball project
# http://icculus.org/neverball/
#
# NEVERBALL is  free software; you can redistribute  it and/or modify
# it under the  terms of the GNU General  Public License as published
# by the Free  Software Foundation; either version 2  of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
# MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
# General Public License for more details.

test -n "$1" || exit 1

LC_ALL=C
export LC_ALL

cd "$1"

DATA="data"
SETS="$DATA/sets.txt"
COURSES="$DATA/courses.txt"

for i in $(cat "$SETS"); do
    i="$DATA/$i"

    # Only translate the two first lines
    head -n 2 $i | while read -r d; do
        echo
        echo "#: $i"
        echo "msgid \"$(echo "$d" | sed 's/\\/\\\\/g')\""
        echo "msgstr \"\""
    done
done

# the "echo | cat x -" forces the end of the last line
echo | cat "$COURSES" - | while read -r d; do
    # Heuristic: description is non empty line without .txt inside
    if test -n "$d" && echo "$d" | grep -v ".txt" &> /dev/null; then
        echo
        echo "#: $COURSES"
        echo "msgid \"$(echo "$d" | sed 's/\\/\\\\/g')\""
        echo "msgstr \"\""
    fi
done

for i in $(find $DATA -name "*.map" | sort); do
    # Only translatable string is "message"
    grep -E "^\"message\"" "$i" | while read -r a b; do
        echo
        echo "#: $i"
        echo "msgid $(echo "$b" | sed 's/\\/\\\\/g')"
        echo "msgstr \"\""
    done
done


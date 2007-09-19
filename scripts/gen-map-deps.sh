#!/bin/sh

test -n "$1" && test -n "$2" || exit 1

MAP=$1
SOL=$(echo $1 | sed -e "s/\.map$/\.sol/")
DATA=$2

OBJS=$(grep -E "^\"model\"" $MAP | cut -d \" -f 4 | sort | uniq | \
    sed -e "s,\\\\,/,g" -e "s,^,$DATA/,")

echo "$SOL :" $OBJS
echo
echo "$OBJS" | sed -e "s/$/ :\n/"


#!/bin/bash
# This program creates the neverball.pot file from source code and level files.

# Copyright (C) 2006 Jean Privat
# Part of the Neverball Project http://icculus.org/neverball/
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

	       
POTFILE="$1"
DOMAIN="$2"
COPYRIGHT="Robert Kooima"
BUGADDR="robert.kooima@gmail.com"

# First, extract from source
echo "# Sources"
> "$POTFILE"
xgettext --from-code=UTF-8 --keyword=_ --keyword=N_ --keyword=sgettext -d "$DOMAIN" --copyright-holder="$COPYRIGHT" --msgid-bugs-address="$BUGADDR" -F -o "$POTFILE" ball/*.[ch] putt/*.[ch] share/*.[ch]

# Force encoding to UTF-8
sed -i "s/charset=CHARSET/charset=UTF-8/" "$POTFILE"

# Second, extract from neverball sets and neverputt courses
echo "# Sets and courses"
for i in data/sets.txt data/courses.txt; do
	# the "echo | cat x -" forces the end of the last line
	echo | cat "$i" - | while read -r d; do
		# Heuristic: description is non empty line without .txt inside
		if test -n "$d" && echo "$d" | grep -v ".txt" > /dev/null 2> /dev/null; then
			echo
			echo "#: $i"
			# Convert \ to \\ 
			echo "msgid \"${d//\\/\\\\}\""
			echo "msgstr \"\""
		fi
	done >> $POTFILE
done

# Third, extracts from levels
echo -n "# Levels: "
find data -name "*.map" | sort | tee .map_list | wc -l
for i in `cat .map_list`; do
	# Check encoding?
	# file --mime $i
	echo -n '.'
	# Only translatable string is "message"
	grep -E "^\"message\"" "$i" | while read -r a b; do
		echo
		echo "#: $i"
		echo "msgid ${b//\\/\\\\}"
		echo "msgstr \"\""
	done >> $POTFILE
done
echo " ok"
rm .map_list

# Remove duplicates, to keep msgmerge from complaining
echo "# Removing duplicates."
msguniq -o "$POTFILE" -t UTF-8 "$POTFILE"


#!/bin/bash
# This program creates the neverball.pot file from source code and level files.

POTFILE="$1"
DOMAIN="$2"
COPYRIGHT="Robert Kooima"
BUGADDR="robert.kooima@gmail.com"

# First, extract from source
echo "# Sources"
> "$POTFILE"
xgettext --from-code=UTF-8 --keyword=_ --keyword=N_ -d "$DOMAIN" --copyright-holder="$COPYRIGHT" --msgid-bugs-address="$BUGADDR" -F -o "$POTFILE" ball/*.[ch] putt/*.[ch] share/*.[ch]

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
echo "# Levels"
for i in `find data -name "*.map"`; do
	# Check encoding?
	# file --mime $i
	
	# Only translatable string is "message"
	grep -E "^\"message\"" "$i" | while read -r a b; do
		echo
		echo "#: $i"
		echo "msgid ${b//\\/\\\\}"
		echo "msgstr \"\""
	done >> $POTFILE
done

# Remove duplicates, to keep msgmerge from complaining
echo "Removing duplicates."
msguniq -o "$POTFILE" -t UTF-8 "$POTFILE"


#!/bin/sh

level=$1
scores=$2
author=$3
tag=$4

cat $level | while read sol back shot grad time goal song; do
	echo -n "$file: "
	file=data/${sol/.sol/}
	name=`basename $file`
	echo "name=$name$tag"

	grep $name $scores > .t
	read t0 c0 t1 c1 t2 c2 rest < .t
	rm .t
	echo "scores $t0 $c0 $t1 $c1 $t2 $c2"

	if [ "$c2" == "$goal" ]; then
		c2=$c1
	fi

	if grep -q '"back"' $file.map; then
		echo skip ;
	else
		echo ok ;
		ed $file.map <<FIN
/"worldspawn"/
a
"author"    "$author"
"levelname" "$name$tag"
"version"   "1"

"back"      "$back"
"grad"      "$grad"
"shot"      "$shot"
"song"      "$song"
"time"      "$time"
"goal"      "$goal"

"time_hs"   "$t0 $t2"
"coin_hs"   "$c0 $c2"
.
w
q
FIN
	fi
done

#!/bin/sh

[ "$1" -a "$2" -a "$3" ] || {
	echo "Usage: $(basename $0) [package-id] [set-easy.txt] [outdir]"
	exit 1
}

id="$1"
setfile="$2"
outdir="$3"

zipfile="${id}.zip" &&
sols="$(sed -n '6,$ p' < "$setfile")" &&
setshot="$(sed -n '4 p' < "$setfile")" &&
setname="$(sed -n '1 p' < "$setfile")" &&
setdesc="$(sed -n '2 p' < "$setfile")" &&
setshotdir="$(echo "$setshot" | sed 's,/.*$,,')" &&
( [ -f sets.txt ] && zip -q -r "$zipfile" $sols "$setfile" "$setshotdir" || zip -q -r "$zipfile" . ) && # build zip file
packagesize="$(du -b "$zipfile" | cut -f1)" &&
checksum=$(md5sum "$zipfile" | cut -d' ' -f1) &&
packagefile="${id}-${checksum}.zip" && # construct package filename
packageshot="${id}-${checksum}.${setshot##*.}" # construct package image filename
cp "$setshot" "${outdir}/$packageshot" && # install package image
mv "$zipfile" "${outdir}/${packagefile}" && # install package zip
cat <<EOF
package $id
filename $packagefile
size $packagesize
files $setfile
name $setname
desc $setdesc
shot $packageshot

EOF
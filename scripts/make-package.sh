#!/bin/sh

# First off, ZIP file determinism:
#
# I had the brilliant idea of adding MD5 checksums to ZIP filenames,
# as a hassle-free way of versioning packages. It is supposed to
# be a content-based checksum: if the ZIP contents are different,
# the checksums are different, so the game knows the packages are
# different and queues an update.
#
# This makes sense, but there's a problem: ZIP files do not only
# store the contents of files, but also file metadata, notably
# file timestamps. So if you recompile your SOLs or just do
# a git checkout on your files, the resulting ZIP file will have
# a different checksum.
#
# A solution to this is to create a copy of the packaged set
# of files and set their timestamps to a fixed date and time.
# Also, "extended" attributes must be excluded from the ZIP file.
#
# So, it's not actually hassle-free.
touch_files() {
    # May 1 2003 is puttnik-0501 release date.
    find . \( -type f -o -type d \) -execdir touch -t 200305010000 {} \;
}

# Given "/tmp/destdir/" and "map-easy/easy.sol", create a copy at
# "/tmp/destdir/map-easy/easy.sol".
install_with_dirs() {
    # First parameter is the destination directory.
    destdir="$1"
    shift

    # Remaining parameters are the source files.
    for f in "$@"; do
        d="$(dirname "$f")" &&
        mkdir -p "${destdir}/${d}" &&
        cp "$f" "${destdir}/${d}/" # important trailing slash
    done
}

[ "$1" -a "$2" -a "$3" ] || {
    echo "Usage: $(basename $0) [package-id] [set-easy.txt] [outdir]"
    exit 1
}

id="$1"
setfile="$2"
outdir="$3"

zipfile="${id}.zip"

# Create the build directory.
tempdir="$(mktemp -d)" || exit

sols="$(sed -n '6,$ p' < "$setfile")" &&
setshot="$(sed -n '4 p' < "$setfile")" &&
setname="$(sed -n '1 p' < "$setfile")" &&
setdesc="$(sed -n '2 p' < "$setfile")" &&
setshotdir="$(dirname "$setshot")" &&
if [ -f sets.txt ]; then
    cp "$setfile" "$tempdir" && # install set file
    cp -r "$setshotdir" "$tempdir" && # install level shots
    install_with_dirs "$tempdir" $sols # install sols
else
    cp -r * "$tempdir"/ && # install everything
    find "$tempdir" \( -name '*.map' -o -name 'Thumbs.db' \) -delete # but delete some things
fi &&
(cd "$tempdir" && touch_files && zip -X -q -r "$zipfile" .) && # build zip file
packagesize="$(du -b "$tempdir/$zipfile" | cut -f1)" &&
checksum=$(md5sum "$tempdir/$zipfile" | cut -d' ' -f1) &&
shotchecksum=$(md5sum "$tempdir/$setshot" | cut -d' ' -f1) &&
packagefile="${id}-${checksum}.zip" && # construct package filename
packageshot="${id}-${shotchecksum}.${setshot##*.}" # construct package image filename
cp "$tempdir/$setshot" "${outdir}/$packageshot" && # install package image
mv "$tempdir/$zipfile" "${outdir}/${packagefile}" && # install package
cat <<EOF
package $id
filename $packagefile
size $packagesize
files $setfile
name $setname
desc $setdesc
shot $packageshot

EOF

# Remove the build directory.
rm -rf "$tempdir"
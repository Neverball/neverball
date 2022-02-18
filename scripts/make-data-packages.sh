#!/bin/sh

data_dir="$1"
target_dir="$2"
make_package="$3"

if cd "$data_dir"; then
    for set in $(cat sets.txt); do
        if ! [ "$set" = "set-easy.txt" -o "$set" = "set-misc.txt" ]; then
            package_id="$(basename "$set" .txt)" &&
            "$make_package" "$package_id" "$set" "$target_dir"
        fi
    done
fi
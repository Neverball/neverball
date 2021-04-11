#!/bin/sh

package_repo_dir="$1"
target_dir="$2"
make_package="$3"

if cd "$package_repo_dir"; then
    for pkg in set-*; do
        if cd "$pkg"; then
            package_id="$pkg"
            "$make_package" "$package_id" "${pkg}.txt" "$target_dir"
            cd ..
        fi
    done
fi
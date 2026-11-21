#!/bin/sh

# Compute a content hash for the paths listed on the command line.

[ "$#" -gt 0 ] || {
    echo "Usage: $0 paths..."
    exit 1
}

# SOLs are non-deterministic, so ignore them when hashing file contents.
# We're still hashing *.map files, so no information should be lost.
# TODO: this still gives different results on different operating systems
# due to line endings. Easy solution: never use this on Windows, Git uses
# LF line endings everywhere else.
HASH="$(find "$@" -type f -not -name '*.sol' | LC_ALL=C sort | tr '\n' '\0' | xargs -0 md5sum | cut -c -32 | md5sum - | cut -c -32)"

echo "$HASH"
#!/bin/sh

# Poor man's ./configure

test "$CC" || {
    echo "Set the CC env var before running this script." >&2
    exit 1
}

trg=sse-test
src=$trg.c

trap "rm -f $trg $src" EXIT

cat > $src <<EOF
#if __i386__
int main(void) { return 0; }
#else
#error
#endif
EOF

# Check whether we're compiling for x86, exit if not.
$CC -o $trg $src 2> /dev/null || exit 0

# Request SSE math.
flags="-mfpmath=sse"

# Check whether additional flags are needed.
if ! $CC "$flags" -Werror -o $trg $src 2> /dev/null; then
    flags="-msse $flags"
fi

echo "$flags"

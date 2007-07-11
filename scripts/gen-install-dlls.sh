#!/bin/sh

if test -z "$PREFIX"; then
    cat >&2 <<EOF
PREFIX not set.  Set the PREFIX environment variable to your MinGW path,
such as /mingw, /usr/i586-mingw32msvc or ~/mingw.
EOF
    exit 1
fi

if ! test -d "$PREFIX"; then
    cat >&2 <<EOF
PREFIX not valid.  Make sure the PREFIX environment variable is set to a
valid location.
EOF
    exit 1
fi

cat <<EOF
#!/bin/sh
# This script was generated automatically.
# Edit the list below to suit your needs.

install \\
$(ls "$PREFIX"/{bin,lib}/*.dll | sed -e 's/^/    /' -e 's/$/ \\/')
    .
EOF


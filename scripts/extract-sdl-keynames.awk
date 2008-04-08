BEGIN {
    print "static const char *table[][2] = {"
}

/\tkeynames\[SDLK_[^\]]+] =/ {
    if (match($0, /\[[^\]]+\]/))
        key = substr($0, RSTART + 1, RLENGTH - 2)

    if (match($0, /"[^"]+"/))
        name = substr($0, RSTART + 1, RLENGTH - 2)

    if (key && name)
    {
        print "    { \"" name "\", " "N_(\"" name "\") },"
    }
}

END {
    print "};"
}


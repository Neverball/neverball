#ifndef SUBSTR_H
#define SUBSTR_H 1

#include <string.h>
#include "common.h"
#include "strbuf.h"

static inline struct strbuf substr(const char *str, size_t start, size_t count)
{
    struct strbuf sb = { "" };

    if (str)
    {
        const size_t max_start = strlen(str);

        start = MIN(start, max_start);
        count = MIN(count, max_start - start);
        count = MIN(count, sizeof (sb.buf) - 1u);

        if (count > 0)
        {
            memcpy(sb.buf, str + start, count);
            sb.buf[count] = 0;
        }
    }

    return sb;
}

/*
 * Allocate a fixed-size buffer on the stack and fill it with the given substring.
 */
#define SUBSTR(str, start, count) (substr((str), (start), (count)).buf)

#endif
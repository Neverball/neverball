#ifndef STRBUF_H
#define STRBUF_H 1

#include <string.h>
#include "common.h"

struct strbuf
{
    char buf[256];
};

typedef struct strbuf STRBUF;

#define STRBUF_WRAP(fn) \
    static inline STRBUF fn ## _strbuf(const char *input) \
    { \
        STRBUF sb = { "" }; \
        const char *output = fn(input); \
        if (output) \
        { \
            const size_t len = MIN(strlen(output), sizeof (sb.buf) - 1u); \
            memcpy(sb.buf, output, len); \
            sb.buf[len] = 0; \
        } \
        return sb; \
    }

static inline STRBUF strbuf(const char *input)
{
    STRBUF sb = { "" };
    const size_t len = MIN(strlen(input), sizeof (sb.buf) - 1u);
    memcpy(sb.buf, input, len);
    sb.buf[len] = 0;
    return sb;
}

/*
 * Convert a STRBUF to a char pointer.
 * The address-of operator &(sb) forces a compilation error if sb is an rvalue
 * temporary (e.g. returned by value from a function), preventing use-after-scope
 * dangling pointer bugs.
 */
#define CSTR(sb) ((void)&(sb), (sb).buf)

#endif

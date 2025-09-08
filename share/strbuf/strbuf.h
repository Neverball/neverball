#ifndef STRBUF_H
#define STRBUF_H 1

#include <string.h>
#include "common.h"

struct strbuf
{
    char buf[64];
};

#define STRBUF_WRAP(fn) \
    inline struct strbuf fn ## _strbuf(const char *input) \
    { \
        struct strbuf sb = { "" }; \
        const char *output = fn(input); \
        if (output) \
        { \
            const size_t len = MIN(strlen(output), sizeof (sb.buf) - 1u); \
            memcpy(sb.buf, output, len); \
            sb.buf[len] = 0; \
        } \
        return sb; \
    }

inline struct strbuf strbuf(const char *input)
{
    struct strbuf sb = { "" };
    const size_t len = MIN(strlen(input), sizeof (sb.buf) - 1u);
    memcpy(sb.buf, input, len);
    sb.buf[len] = 0;
    return sb;
}

#define STR(sb) ((sb).buf)

#endif

#ifndef JOINSTR_H
#define JOINSTR_H 1

#include <string.h>
#include "common.h"
#include "strbuf.h"

static inline STRBUF joinstr(const char *head, const char *tail)
{
    STRBUF sb = { "" };

    const size_t max_len = sizeof (sb.buf) - 1u;
    const size_t head_len = head ? MIN(strlen(head), max_len) : 0u;
    const size_t tail_len = tail ? MIN(strlen(tail), max_len - head_len) : 0u;

    if (head_len > 0)
        memcpy(sb.buf, head, head_len);

    if (tail_len > 0)
        memcpy(sb.buf + head_len, tail, tail_len);

    sb.buf[head_len + tail_len] = 0;

    return sb;
}

#endif
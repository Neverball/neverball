#ifndef BASE_NAME_H
#define BASE_NAME_H 1

#include "strbuf.h"
#include "common.h"

STRBUF_WRAP(base_name)

/*
 * Allocate a fixed-size buffer on the stack and fill it with the base name of
 * the given path.
 */
#define BASE_NAME(name) (base_name_strbuf((name)).buf)

#endif
#ifndef DIR_NAME_H
#define DIR_NAME_H 1

#include "strbuf.h"
#include "common.h"

STRBUF_WRAP(dir_name)

/*
 * Allocate a fixed-size buffer on the stack and fill it with the dir name
 * of the given path.
 */
#define DIR_NAME(name) (dir_name_strbuf((name)).buf)

#endif
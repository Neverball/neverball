#ifndef DIR_H
#define DIR_H

#include "array.h"
#include "list.h"

struct dir_item
{
    const char *path;
    void *data;
};

/*
 * A minor convenience for quick member access, as in "DIR_ITEM_GET(a,
 * i)->member".  Most of the time this macro is not what you want to
 * use.
 */
#define DIR_ITEM_GET(a, i) ((struct dir_item *) array_get((a), (i)))

Array dir_scan(const char *,
               int  (*filter)    (struct dir_item *),
               List (*list_files)(const char *),
               void (*free_files)(List));
void  dir_free(Array);

List dir_list_files(const char *);
void dir_list_free (List);

int dir_exists(const char *);

#ifdef _WIN32
#include <direct.h>
#define dir_make(path) _mkdir(path)
#else
#include <sys/stat.h>
#define dir_make(path) mkdir(path, 0777)
#endif

#endif

#include <dirent.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "dir.h"
#include "common.h"

static struct dir_item *add_item(Array items, const char *dir, const char *name)
{
    struct dir_item *item = array_add(items);

    item->path = concat_string(dir, "/", name, NULL);
    item->data = NULL;

    return item;
}

static void del_item(Array items)
{
    struct dir_item *item = array_get(items, array_len(items) - 1);

    free((void *) item->path);
    assert(!item->data);

    array_del(items);
}

/*
 * Scan the directory PATH and return an array of dir_item structures
 * for which FILTER evaluates to non-zero (or all, if FILTER is NULL).
 * FILTER can associate data with a dir_item for later use by
 * assigning it to the "data" member.
 */
Array dir_scan(const char *path, int (*filter)(struct dir_item *))
{
    DIR *dir;
    Array items = NULL;

    /*
     * HACK: MinGW provides numerous POSIX extensions to MSVCRT,
     * including dirent.h, so parasti ever so lazily has not bothered
     * to port the code below to FindFirstFile et al.
     */

    if ((dir = opendir(path)))
    {
        struct dirent *ent;

        items = array_new(sizeof (struct dir_item));

        while ((ent = readdir(dir)))
        {
            struct dir_item *item;

            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            item = add_item(items, path, ent->d_name);

            if (filter && !filter(item))
                del_item(items);
        }

        closedir(dir);
    }

    return items;
}

void dir_free(Array items)
{
    while (array_len(items))
        del_item(items);
}

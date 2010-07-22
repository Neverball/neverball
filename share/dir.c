#include <dirent.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "dir.h"
#include "common.h"

static char **get_dir_list(const char *path)
{
    DIR *dir;
    char **files = NULL;
    int count = 0;

    /*
     * HACK: MinGW provides numerous POSIX extensions to MSVCRT,
     * including dirent.h, so parasti ever so lazily has not bothered
     * to port the code below to FindFirstFile et al.
     */

    if ((dir = opendir(path)))
    {
        struct dirent *ent;

        files = malloc(sizeof (char *));

        while ((ent = readdir(dir)))
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            files[count++] = strdup(ent->d_name);
            files = realloc(files, (count + 1) * sizeof (char *));
        }

        files[count] = NULL;

        closedir(dir);
    }

    return files;
}

static void free_dir_list(void *files)
{
    if (files)
    {
        char **file;

        /* Free file names. */
        for (file = files; *file; free(*file++));

        /* Free trailing NULL. */
        free(*file);

        /* Free pointer list. */
        free(files);
    }
}

static struct dir_item *add_item(Array items, const char *dir, const char *name)
{
    struct dir_item *item = array_add(items);

    item->path = *dir ? concat_string(dir, "/", name, NULL) : strdup(name);
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

Array dir_scan(const char *path,
               int    (*filter)   (struct dir_item *),
               char **(*get_list) (const char *),
               void   (*free_list)(void *))
{
    char **list;
    Array items = NULL;

    assert((get_list && free_list) || (!get_list && !free_list));

    if (!get_list)
        get_list = get_dir_list;

    if (!free_list)
        free_list = free_dir_list;

    if ((list = get_list(path)))
    {
        char **file = list;

        items = array_new(sizeof (struct dir_item));

        while (*file)
        {
            struct dir_item *item;

            item = add_item(items, path, *file);

            if (filter && !filter(item))
                del_item(items);

            file++;
        }

        free_list(list);
    }

    return items;
}

void dir_free(Array items)
{
    while (array_len(items))
        del_item(items);

    array_free(items);
}

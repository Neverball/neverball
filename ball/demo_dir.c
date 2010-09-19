#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "array.h"
#include "common.h"
#include "demo.h"
#include "demo_dir.h"
#include "fs.h"

/*---------------------------------------------------------------------------*/

static void free_item(struct dir_item *item)
{
    if (item->data)
    {
        demo_free(item->data);
        item->data = NULL;
    }
}

static void load_item(struct dir_item *item)
{
    if (!item->data)
        item->data = demo_load(item->path);
}

static int scan_item(struct dir_item *item)
{
    return str_ends_with(item->path, ".nbr");
}

/*---------------------------------------------------------------------------*/

Array demo_dir_scan(void)
{
    return fs_dir_scan("Replays", scan_item);
}

void demo_dir_load(Array items, int lo, int hi)
{
    int i;

    assert(lo >= 0  && lo < array_len(items));
    assert(hi >= lo && hi < array_len(items));

    for (i = lo; i <= hi; i++)
        load_item(array_get(items, i));
}

void demo_dir_free(Array items)
{
    int i;

    for (i = 0; i < array_len(items); i++)
        free_item(array_get(items, i));

    dir_free(items);
}

/*---------------------------------------------------------------------------*/

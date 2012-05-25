/*
 * Copyright (C) 2003-2010 Neverball authors
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

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

static int cmp_items(const void *A, const void *B)
{
    const struct dir_item *a = A, *b = B;

    if (strcmp(base_name_sans(a->path, ".nbr"), USER_REPLAY_FILE) == 0)
        return -1;
    if (strcmp(base_name_sans(b->path, ".nbr"), USER_REPLAY_FILE) == 0)
        return +1;

    return strcmp(a->path, b->path);
}

/*---------------------------------------------------------------------------*/

Array demo_dir_scan(void)
{
    Array items;

    if ((items = fs_dir_scan("Replays", scan_item)))
        array_sort(items, cmp_items);

    return items;
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

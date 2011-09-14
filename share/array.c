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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "array.h"
#include "common.h"

/*----------------------------------------------------------------------------*/

void alloc_new(struct alloc *alloc,
               int block,
               void **data,
               int   *count)
{
    memset(alloc, 0, sizeof (*alloc));

    alloc->data  = data;
    alloc->count = count;

    *alloc->data  = NULL;
    *alloc->count = 0;

    alloc->size   = 0;
    alloc->block  = block;
}

void alloc_free(struct alloc *alloc)
{
    if (alloc->data)
    {
        free(*alloc->data);
        *alloc->data = NULL;
    }

    if (alloc->count)
    {
        *alloc->count = 0;
    }
}

void *alloc_add(struct alloc *alloc)
{
    if ((*alloc->count + 1) * alloc->block > alloc->size)
    {
        void *new_data;
        int   new_size = (alloc->size > 0 ?
                          alloc->size * 2 :
                          alloc->block);

        if ((new_data = realloc(*alloc->data, new_size)))
        {
            *alloc->data = new_data;
            alloc->size  = new_size;
        }
        else
            return NULL;
    }

    return (((unsigned char *) *alloc->data) +
            ((*alloc->count)++ * alloc->block));
}

void alloc_del(struct alloc *alloc)
{
    if (*alloc->count > 0)
    {
        if ((*alloc->count - 1) * alloc->block == alloc->size / 4)
            *alloc->data  = realloc(*alloc->data, (alloc->size /= 4));

        (*alloc->count)--;
    }
}

/*----------------------------------------------------------------------------*/

struct array
{
    unsigned char *data;

    int elem_num;
    int elem_len;

    struct alloc alloc;
};

Array array_new(int elem_len)
{
    Array a;

    assert(elem_len > 0);

    if ((a = malloc(sizeof (*a))))
    {
        a->elem_num = 0;
        a->elem_len = elem_len;

        alloc_new(&a->alloc, elem_len, (void **) &a->data, &a->elem_num);
    }

    return a;
}

void array_free(Array a)
{
    assert(a);

    alloc_free(&a->alloc);
    free(a);
}

void *array_add(Array a)
{
    assert(a);

    return alloc_add(&a->alloc);
}

void array_del(Array a)
{
    assert(a);
    assert(a->elem_num > 0);

    alloc_del(&a->alloc);
}

void *array_get(Array a, int i)
{
    assert(a);
    assert(i >= 0 && i < a->elem_num);

    return &a->data[i * a->elem_len];
}

void *array_rnd(Array a)
{
    assert(a);

    return a->elem_num ? array_get(a, rand_between(0, a->elem_num - 1)) : NULL;
}

int array_len(Array a)
{
    assert(a);

    return a->elem_num;
}

void array_sort(Array a, int (*cmp)(const void *, const void *))
{
    assert(a);

    qsort(a->data, a->elem_num, a->elem_len, cmp);
}

/*----------------------------------------------------------------------------*/

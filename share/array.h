#ifndef ARRAY_H
#define ARRAY_H

/*----------------------------------------------------------------------------*/

/*
 * Array allocator that minimizes allocations.
 */

struct alloc
{
    void **data;
    int   *count;

    int size;
    int block;
};

void  alloc_new(struct alloc *, int, void **, int *);
void  alloc_free(struct alloc *);
void *alloc_add(struct alloc *);
void  alloc_del(struct alloc *);

/*----------------------------------------------------------------------------*/

typedef struct array *Array;

Array array_new(int);
void  array_free(Array);

void *array_add(Array);
void  array_del(Array);
void *array_get(Array, int);
void *array_rnd(Array);
int   array_len(Array);

void  array_sort(Array, int (*cmp)(const void *, const void *));

/*----------------------------------------------------------------------------*/

#endif

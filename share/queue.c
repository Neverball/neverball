/*
 * Copyright (C) 2009 Neverball authors
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
#include <assert.h>
#include "queue.h"
#include "list.h"

struct queue
{
    List head;
    List tail;
};

Queue queue_new(void)
{
    Queue new;

    if ((new = malloc(sizeof (*new))))
    {
        if (!(new->head = new->tail = list_cons(NULL, NULL)))
        {
            free(new);
            return NULL;
        }
    }

    return new;
}

void queue_free(Queue q)
{
    if (!q)
        return;

    assert(queue_empty(q));
    free(q->head);
    free(q);
}

int queue_empty(Queue q)
{
    if (!q || !q->head)
        return 1;
    return q->head == q->tail;
}

void queue_enq(Queue q, void *data)
{
    List next_node;

    if (!q || !q->tail)
        return;

    if ((next_node = list_cons(NULL, NULL)))
    {
        q->tail->data = data;
        q->tail->next = next_node;
        q->tail = next_node;
    }
}

void *queue_deq(Queue q)
{
    void *data = NULL;

    if (!queue_empty(q) && q->head)
    {
        data    = q->head->data;
        q->head = list_rest(q->head);
    }

    return data;
}

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

#include "game_proxy.h"
#include "queue.h"
#include "cmd.h"

static Queue cmd_queue;

/*
 * Enqueue SRC in the game's command queue.
 */
void game_proxy_enq(const union cmd *src)
{
    union cmd *dst;

    /*
     * Create the queue.  This is done only once during the life time
     * of the program.  For simplicity's sake, the queue is never
     * destroyed.
     */

    if (!cmd_queue)
        cmd_queue = queue_new();

    /*
     * Add a copy of the command to the end of the queue.
     */

    if ((dst = malloc(sizeof (*dst))))
    {
        *dst = *src;
        queue_enq(cmd_queue, dst);
    }
}

/*
 * Dequeue and return the head element in the game's command queue.
 * The element must be freed after use.
 */
union cmd *game_proxy_deq(void)
{
    return cmd_queue ? queue_deq(cmd_queue) : NULL;
}

/*
 * Clear the entire queue.
 */
void game_proxy_clr(void)
{
    union cmd *cmdp;

    while ((cmdp = game_proxy_deq()))
        cmd_free(cmdp);
}

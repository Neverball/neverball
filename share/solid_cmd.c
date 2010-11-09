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

#include "solid_cmd.h"
#include "cmd.h"

/*---------------------------------------------------------------------------*/

static void (*cmd_enq_fn)(const union cmd *);

void sol_cmd_enq(const union cmd *new)
{
    if (cmd_enq_fn)
        cmd_enq_fn(new);
}

void sol_cmd_enq_func(void (*enq_fn) (const union cmd *))
{
    cmd_enq_fn = enq_fn;
}

/*---------------------------------------------------------------------------*/

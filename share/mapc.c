/*
 * Copyright (C) 2003 Robert Kooima
 * Copyright (C) 2025 Jānis Rūcis
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

/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>

#include "fs.h"
#include "mapclib.h"

int main(int argc, char *argv[])
{
    mapc_context ctx = NULL;

    if (!fs_init(argc > 0 ? argv[0] : NULL))
    {
        fprintf(stderr, "Failure to initialize virtual file system: %s\n", fs_error());
        return 1;
    }

    mapc_init(&ctx);

    if (mapc_opts(ctx, argc, argv))
    {
        if (mapc_compile(ctx))
        {
            mapc_dump(ctx);
            mapc_quit(&ctx);
            exit(EXIT_SUCCESS);
        }
        else
        {
            mapc_quit(&ctx);
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}


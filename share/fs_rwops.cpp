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

#include "fs_rwops.h"

static Sint64 rwops_seek(SDL_RWops *ctx, Sint64 offset, int whence)
{
    fs_file fh = (fs_file)ctx->hidden.unknown.data1;
    return fs_seek(fh, offset, whence) ? fs_tell(fh) : -1;
}

static size_t rwops_read(SDL_RWops *ctx, void *ptr, size_t size, size_t maxnum)
{
    return fs_read(ptr, size, maxnum, (fs_file)ctx->hidden.unknown.data1);
}

static size_t rwops_write(SDL_RWops *ctx, const void *ptr, size_t size, size_t num)
{
    return fs_write(ptr, size, num, (fs_file)ctx->hidden.unknown.data1);
}

static int rwops_close(SDL_RWops *ctx)
{
    fs_file fh = (fs_file)ctx->hidden.unknown.data1;

    if (!fs_close(fh))
        return -1;

    SDL_FreeRW(ctx);
    return 0;
}

SDL_RWops *fs_rwops_make(fs_file fh)
{
    SDL_RWops *ctx;

    if ((ctx = SDL_AllocRW()))
    {
        ctx->seek  = rwops_seek;
        ctx->read  = rwops_read;
        ctx->write = rwops_write;
        ctx->close = rwops_close;

        ctx->hidden.unknown.data1 = fh;
    }

    return ctx;
}

SDL_RWops *fs_rwops_open(const char *path, const char *mode)
{
    fs_file fh;

    if ((fh = fs_open(path, mode)))
        return fs_rwops_make(fh);

    return NULL;
}

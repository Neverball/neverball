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

#include "fs.h"
#include "fs_ov.h"

size_t fs_ov_read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return fs_read(ptr, size, nmemb, datasource);
}

int fs_ov_seek(void *datasource, ogg_int64_t offset, int whence)
{
    return fs_seek(datasource, offset, whence);
}

int fs_ov_close(void *datasource)
{
    return fs_close(datasource);
}

long fs_ov_tell(void *datasource)
{
    return fs_tell(datasource);
}

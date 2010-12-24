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

#include <png.h>
#include <string.h>
#include "fs_png.h"
#include "fs.h"

/*---------------------------------------------------------------------------*/

void fs_png_read(png_structp readp, png_bytep data, png_size_t length)
{
    int read = fs_read(data, 1, length, png_get_io_ptr(readp));

    if (read < length)
        memset(data + read, 0, length - read);
}

void fs_png_write(png_structp writep, png_bytep data, png_size_t length)
{
    fs_write(data, 1, length, png_get_io_ptr(writep));
}

void fs_png_flush(png_structp writep)
{
    fs_flush(png_get_io_ptr(writep));
}

/*---------------------------------------------------------------------------*/
